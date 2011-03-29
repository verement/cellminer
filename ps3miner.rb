
require 'rubygems'

require 'optparse'
require 'pp'

require './bitcoin'
require './ext/spu_miner'

class PS3Miner
  attr_reader :options, :client

  def initialize(args)
    @options = {
      :threads => 1
    }
    OptionParser.new do |opts|
      opts.banner = "Usage: #{$0} [options] [rpc-server-host]"

      opts.on("-t", "--threads N", Integer,
              "Number of SPU threads to start") do |n|
        @options[:threads] = n
      end

      opts.on("-b", "--balance",
              "Show balance and exit") do |opt|
        @options[:balance] = opt
      end
    end.parse!(args)

    server = ARGV.shift || ENV['BITCOIN_SERVER'] ||
      begin
        warn "Warning: using default server localhost"
        'localhost'
      end
    @client = Bitcoin.rpc_client(:host => server)
  end

  def main
    if options[:balance]
      puts "Current balance = #{client.getbalance} BTC"
      exit
    end

    puts "Current block count is #{client.getblockcount}"
    puts "Current difficulty is #{client.getdifficulty}"

    puts "Starting #{options[:threads]} mining thread(s)..."

    @mutex = Mutex.new

    threads = []
    options[:threads].times do |n|
      threads << Thread.new { mine(n) }
    end

    threads.each {|thread| thread.join }
  end

  def mine(n)
    miner = Bitcoin::SPUMiner.new

    # unpack hex strings and fix byte ordering
    work = Hash[client.getwork.map {|k, v|
                  [k.to_sym, [v].pack('H*').unpack('V*').pack('N*')]}]

    if solved = miner.run(work[:data], work[:target],
                          work[:midstate], work[:hash1])
      @mutex.synchronize do
        puts "Solved? data ="
        pp solved
      end
      # send back to server...
    end
  end
end
