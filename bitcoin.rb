
require 'uri'
require 'json'
require 'rest_client'

module Bitcoin
  CONFFILE = File.join(ENV['HOME'], '.bitcoin', 'bitcoin.conf')

  def self.conf
    parms = File.read(CONFFILE).split("\n")
    parms.inject(Hash.new) {|h, p| h.merge! Hash[*p.split("=")] }
  end

  def self.rpc_client(server = {})
    conf = self.conf
    default = {
      :userinfo => [conf['rpcuser'], conf['rpcpassword']].join(':'),
      :host => 'localhost',
      :port => 8332,
      :path => '/'
    }
    uri = URI::HTTP.build(default.merge(server))
    RPCClient.new(uri)
  end

  class RPCClient
    attr_reader :url

    class JSONRPCException < RuntimeError; end

    def initialize(uri)
      case uri
      when URI
      when String
        uri = URI.parse(uri)
      else
        raise ArgumentError, "bad URI given"
      end

      @url = uri.to_s
    end

    def method_missing(method, *params)
      postdata = {
        :id => 'jsonrpc',
        :method => method,
        :params => params
      }.to_json
      respdata = RestClient.post(url, postdata)

      resp = JSON.parse(respdata)
      if resp['error']
        raise JSONRPCException, resp['error']
      end

      if block_given?
        yield resp['result']
      else
        resp['result']
      end
    end
  end
end
