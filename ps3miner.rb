
require 'optparse'
require 'pp'

require './bitcoin'
require './ext/spu_miner'

require './sha256'

class PS3Miner
  attr_accessor :options
  attr_reader :rpc

  def initialize(argv = [])
    @options = Hash.new

    options[:threads] = 1
    options[:debug] = ENV['DEBUG']

    OptionParser.new do |opts|
      opts.banner = "Usage: #{$0} [options] [server]"

      opts.on("-t", "--threads N", Integer,
              "Number of SPU threads to start") do |n|
        options[:threads] = n
      end

      opts.on("-b", "--balance",
              "Show balance and exit") do |opt|
        options[:balance] = opt
      end

      opts.on("-t", "--test",
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

    @rpc = Bitcoin.rpc_proxy(:host => server)

    @mutex = Mutex.new
  end

  def main
    if options[:balance]
      puts "Current balance = #{rpc.getbalance} BTC"
      exit
    end

    puts "Current block count is #{rpc.getblockcount}"
    puts "Current difficulty is #{rpc.getdifficulty}"

    puts "Starting #{options[:threads]} mining thread(s)..."

    threads = []
    options[:threads].times do
      threads << Thread.new { mine }
    end

    threads.each {|thread| thread.join }
  end

  private

  attr_reader :mutex

  def say(info)
    mutex.synchronize { puts "#{Thread.current} #{info}" }
  end

  def debug(info)
    say(info) if options[:debug]
  end

  def dump(data, desc)
    mutex.synchronize do
      puts "#{Thread.current} #{desc} ="
      pp data
    end if options[:debug]
    data
  end

  def mine
    miner = Bitcoin::SPUMiner.new(options[:debug])

    debug "Starting with #{miner}"

    loop do
      # unpack hex strings and fix byte ordering
      work = Hash[getwork.map {|k, v|
                    [k.to_sym, [v].pack('H*').unpack('V*').pack('N*')]}]

      if solved = miner.run(work[:data], work[:target],
                            work[:midstate], work[:hash1])
        say "Worker found something!"
        # send back to server...
        response = nil;  # rpc.getwork(solved)

        mutex.synchronize do
          puts "Solved? data ="
          pp solved
          puts "Server response ="
          pp response
        end
      else
        say "Worker found nothing"
      end

      # only one iteration for now
      return
    end
  end

  def getwork
    work = options[:test] ? testwork : rpc.getwork
    dump work, "work"
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
