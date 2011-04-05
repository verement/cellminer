
require 'optparse'
require 'thread'
require 'timeout'
require 'pp'

require './bitcoin.rb'
require './ext/spu_miner.so'

require './sha256.rb'

class CellMiner
  include Timeout

  attr_accessor :options
  attr_reader :rpc

  class AbortMining < StandardError; end

  USER_AGENT = "Cell Miner"

  NSLICES = 128
  QUANTUM = 0x100000000 / NSLICES

  TIMEOUT = 60

  def initialize(argv = [])
    @options = Hash.new

    options[:num_spu] = 6
    options[:debug] = ENV['DEBUG']

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

      opts.on("-s N", Integer,
              "Number of SPUs to use") do |n|
        options[:num_spu] = n
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

    host, port = server.split(':')

    params = {:host => host}
    params[:port] = port.to_i if port
    params[:username] = options[:username] if options[:username]
    params[:password] = options[:password] if options[:password]

    @rpc = Bitcoin.rpc_proxy(params, USER_AGENT)
  end

  def main
    if options[:balance]
      puts "Current balance = #{rpc.getbalance} BTC"
      exit
    end

    work_queue = Queue.new
    solved_queue = Queue.new

    puts "Creating %d SPU miner(s)..." % options[:num_spu]

    Thread.abort_on_exception = true

    miners = []
    options[:num_spu].times do
      miners << Thread.new do
        miner = Bitcoin::SPUMiner.new(options[:debug])

        loop do
          begin
            work = work_queue.shift
            debug "#{miner} Mining %08x..%08x\n" %
              [work[:start_nonce], work[:start_nonce] + work[:range] - 1]
            start = Time.now
            if solution = miner.run(work[:data], work[:target], work[:midstate],
                                    work[:start_nonce], work[:range])
              debug "#{miner} Found solution\n"
              solved_queue << solution
            else
              Thread.current[:rate] = work[:range] / (Time.now - start)
            end
          rescue AbortMining
          end
        end
      end
    end

    last_block  = nil
    last_target = nil

    loop do
      # get work, unpack hex strings and fix byte ordering
      work = getwork

      prev_block = work[:data][4..35].reverse.unpack('H*').first
      target = work[:target].unpack('H*').first

      rate = miners.inject(0) {|sum, thr| sum + (thr[:rate] || 0) }

      say "Got work... %.3f Mhash/s" % (rate / 1_000_000)

      if prev_block != last_block
        say "    prev = %s" % prev_block
        last_block = prev_block

        work_queue.clear
        miners.each {|thr| thr.raise AbortMining }
      end

      if target != last_target
        say "  target = %s" % target
        last_target = target
      end

      work[:range] = QUANTUM

      NSLICES.times do |i|
        work[:start_nonce] = i * work[:range]
        work_queue << work.dup
      end

      begin
        solution = nil
        timeout(TIMEOUT) { solution = solved_queue.shift }

        # send back to server...
        response = sendwork(solution)

        say "=> Solved? (%s)\n    hash = %s\n  target = %s" %
          [response, block_hash(solution), target]

        work_queue.clear
      rescue Timeout::Error
        debug "Timeout"
      end

      return if options[:test]
    end
  end

  private

  def say(info)
    puts info
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

  def getwork
    work = options[:test] ? testwork : rpc.getwork
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
    rpc.getwork(data) rescue nil
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
