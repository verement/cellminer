
require 'rubygems'

require 'optparse'

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

    puts "Starting mining threads..."
    threads = []
    options[:threads].times do |n|
      threads << Thread.new { mine(n) }
    end
    threads.each {|thread| thread.join }
  end

  def mine(n)
    miner = Bitcoin::SPUMiner.new
    miner.run(n)
    miner.run(1000 + n)
  end
end
