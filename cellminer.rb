#
# cellminer - Bitcoin miner for the Cell Broadband Engine Architecture
# Copyright © 2011 Robert Leslie
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License (version 2) as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#

require 'optparse'
require 'uri'
require 'thread'
require 'pp'

require_relative 'bitcoin.rb'
require_relative 'ext/cellminer.so'

require_relative 'sha256.rb'

class CellMiner
  attr_accessor :options
  attr_reader :rpc
  attr_reader :mutex

  class AbortMining < StandardError; end

  USER_AGENT = "Cell Miner"

  NSLICES = 128
  QUANTUM = 0x100000000 / NSLICES

  QUEUE_MAX = 384

  INTERVAL = 60
  RETRY_INTERVAL = 30

  def initialize(argv = [])
    @options = Hash.new

    options[:num_spe] = 6
    options[:num_ppe] = 0
    options[:debug] = $DEBUG

    OptionParser.new do |opts|
      opts.banner = "Usage: #{$0} [options] [server]"

      opts.on("-u", "--username USER", String,
              "RPC Username") do |username|
        options[:username] = username
      end

      opts.on("-p", "--password PASS", String,
              "RPC Password") do |password|
        options[:password] = password
      end

      opts.on("--spe N", Integer,
              "Number of SPEs to use (default %d)" %
              options[:num_spe]) do |opt|
        options[:num_spe] = opt
      end

      opts.on("--ppe N", Integer,
              "Number of PPE threads to use (default %d)" %
              options[:num_ppe]) do |opt|
        options[:num_ppe] = opt
      end

      opts.on("-b", "--balance",
              "Show balance and exit") do |opt|
        options[:balance] = opt
      end

      opts.on("--test",
              "Use known testing data") do |opt|
        options[:test] = opt
      end

      opts.on("-d", "--debug",
              "Show debugging output") do |opt|
        options[:debug] = opt
      end
    end.parse!(argv)

    server = argv.shift || ENV['BITCOIN_SERVER'] ||
      begin
        warn "Warning: using default server localhost"
        'localhost'
      end

    params = {}

    uri = URI.parse(server) rescue nil
    if uri.kind_of? URI::HTTP
      host, port = uri.host, uri.port
      params[:path] = uri.path
    else
      host, port = server.split(':')
    end

    params[:host] = host
    params[:port] = port.to_i if port

    params[:username] = options[:username] if options[:username]
    params[:password] = options[:password] if options[:password]

    @rpc = Bitcoin.rpc_proxy(params, USER_AGENT)
    @mutex = Mutex.new
  end

  def main
    if options[:balance]
      puts "Current balance = #{rpc.getbalance} BTC"
      exit
    end

    say "%s starting" % USER_AGENT

    work_queue = Queue.new
    solved_queue = Queue.new

    miner_proc = lambda do |miner_class|
      miner = miner_class.new(options[:debug])

      loop do
        begin
          start = Time.now
          work = work_queue.shift
          debug "#{miner} Mining %08x..%08x" %
            [work[:start_nonce], work[:start_nonce] + work[:range] - 1]
          if solution = miner.run(work[:data], work[:target], work[:midstate],
                                  work[:start_nonce], work[:range])
            debug "#{miner} Found solution"
            solved_queue << solution
          else
            Thread.current[:rate] = work[:range] / (Time.now - start)
          end
        rescue AbortMining
        end
      end
    end

    Thread.abort_on_exception = true

    miners = ThreadGroup.new
    class << miners
      include Enumerable

      alias << add

      def empty?
        list.empty?
      end

      def each(&block)
        list.each(&block)
      end

      def rate
        inject(0.0) {|sum, thread| sum + (thread[:rate] || 0) }
      end

      def abort
        each {|thread| thread.raise AbortMining }
      end
    end

    if options[:num_spe] > 0
      say "Creating %d SPU miner(s)" % options[:num_spe]
      options[:num_spe].times do
        miners << Thread.new(Bitcoin::SPUMiner, &miner_proc)
      end
    end

    if options[:num_ppe] > 0
      say "Creating %d PPU miner(s)" % options[:num_ppe]
      options[:num_ppe].times do
        miners << Thread.new(Bitcoin::PPUMiner, &miner_proc)
      end
    end

    if miners.empty?
      $stderr.puts "No miners!"
      exit 1
    end

    getwork_queue = Queue.new

    # work processing thread
    Thread.new do
      last_block  = nil
      last_target = nil

      loop do
        work = getwork_queue.shift

        prev_block = work[:data][4..35].reverse.unpack('H*').first
        target = work[:target].unpack('H*').first

        msg = "Got work... %.3f Mhash/s, %d backlogged work items" %
          [miners.rate / 1_000_000, work_queue.length]

        if target != last_target
          last_target = target
          msg << "\n  target = %s" % target
        end

        if prev_block != last_block
          last_block = prev_block
          msg << "\n    prev = %s" % prev_block

          work_queue.clear
          miners.abort
          solved_queue.clear
        end

        say msg

        work[:range] = QUANTUM
        NSLICES.times do |i|
          work[:start_nonce] = i * QUANTUM
          work_queue << work.dup
        end

        # trim excess work
        work_queue.shift(true) while work_queue.length > QUEUE_MAX
      end
    end

    # work gathering thread
    getwork_thread = Thread.new do
      loop do
        getwork_queue << getwork do |poll_rpc|
          unless @long_polling
            @long_polling = true
            say "Starting long poll"

            # long polling thread
            Thread.new do
              loop do
                work = getwork(poll_rpc)
                say "Long poll returned"
                getwork_queue << work
              end
            end
          end
        end

        sleep INTERVAL
      end
    end

    # solution gathering thread
    loop do
      solution = solved_queue.shift

      # send back to server...
      response = sendwork(solution)

      say "Solved? (%s)\n  %s hash = %s" %
        [response, response == true ? '*' : ' ', block_hash(solution)]

      return if options[:test]

      getwork_thread.run if response == false
    end
  end

  private

  def say(info)
    mutex.synchronize do
      puts "[%s] %s" % [Time.now.strftime("%Y-%m-%d %H:%M:%S"), info]
    end
  end

  def debug(info)
    say(info) if options[:debug]
  end

  def dump(data, desc)
    if options[:debug]
      puts "#{desc} ="
      pp data
    end
    data
  end

  def getwork(rpc = rpc, &block)
    begin
      work = options[:test] ? testwork : rpc.getwork(&block)
    rescue => err
      say "RPC error: %s" % err
      sleep RETRY_INTERVAL
      retry
    end

    # unpack hex strings and fix byte ordering
    work = work.map do |k, v|
      k = k.to_sym
      v = [v].pack('H*')
      v = (k == :target) ? v.reverse : v.unpack('V*').pack('N*')
      [k, v]
    end
    Hash[work]
  end

  def sendwork(solution)
    data = solution.unpack('N*').pack('V*').unpack('H*').first

    begin
      rpc.getwork(data)
    rescue => err
      say "RPC error: %s" % err
      sleep RETRY_INTERVAL
      retry
    end
  end

  def block_hash(data)
    hash0 = SHA256.new.update(data)
    hash1 = [UInt256.new(hash0.hexdigest).byte_string,
             0x80000000, 0, 0, 0, 0, 0, 0, 0x100].pack('a32N8')
    hash1 = SHA256.new.update(hash1)
    hash1.digest.reverse.unpack('H*').first
  end

  def testwork
    # Test data for known block (#115558)
    hash = UInt256.new("000000000000bd26aaf867f23a7b66b6"  \
                       "2ffe1e402f79a424ef8b3e4e84c68f32")
    version = 1
    prev_block = UInt256.new("00000000000090b0cae8c5fea2fffac9"  \
                             "f909c097f0402637c59647b91bbd896f")
    merkle_root = UInt256.new("15fbf39fd7120dd06950467fbdf9d226"  \
                              "9ed3f63796b0acab93d89e2888e2b10a")
    time = 1301376879
    bits = 453047097
    nonce = 3406758657

    datav = [version,
            prev_block.reverse_endian.byte_string,
            merkle_root.reverse_endian.byte_string,
            time, bits, nonce,
            0x80000000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x0280]

    check = SHA256.new.update(datav.pack('Va32a32V3N12'))

    hash1 = [UInt256.new(check.hexdigest).byte_string,
             0x80000000, 0, 0, 0, 0, 0, 0, 0x100].pack('a32N8')

    check = SHA256.new.update(hash1)

    dump hash, "supposed hash"
    dump UInt256.new(check.hexdigest).reverse_endian, "calculated hash"

    # zero nonce
    datav[5] = 0
    data = datav.pack('Va32a32V3N12')
    midstate = UInt256.new(SHA256.new.update(data[0..63]).hexdigest)

    target = UInt256.new((bits & 0x00ffffff) * 2**(8 * ((bits >> 24) - 3)))

    # reverse-endian crap
    data     = data.unpack('V*').pack('N*')
    target   = target.reverse_endian.byte_string
    midstate = midstate.byte_string.unpack('V*').pack('N*')
    hash1    = hash1.unpack('V*').pack('N*')

    {
      :data => data.unpack('H*').first,
      :target => target.unpack('H*').first,
      :midstate => midstate.unpack('H*').first,
      :hash1 => hash1.unpack('H*').first
    }
  end
end

class UInt256
  attr_accessor :value

  def initialize(value)
    case value
    when Integer
    when String
      value = pack_chars [value].pack('H*').unpack('C*')
    else
      raise TypeError, "can't cast #{value.class} as #{self.class}"
    end

    @value = value
  end

  def to_s
    "%064x" % value
  end

  def byte_string
    [to_s].pack('H*')
  end

  def reverse_endian
    self.class.new(pack_chars byte_string.unpack('C*').reverse)
  end

  private

  def pack_chars(chars)
    chars.inject(0) {|v, ch| (v << 8) + ch }
  end
end
