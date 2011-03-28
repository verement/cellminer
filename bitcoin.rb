
require 'uri'
require 'rest_client'
require 'json'

module Bitcoin
  def self.conf
    parms = File.read(File.join(ENV['HOME'],
                                '.bitcoin', 'bitcoin.conf')).split("\n")
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
    class JSONRPCException < RuntimeError; end

    def initialize(url)
      case url
      when URI
      when String
        url = URI.parse(url)
      else
        raise ArgumentError, "bad URL given"
      end

      @url = url
    end

    def method_missing(method, *params)
      postdata = {:id => 'jsonrpc',
        :method => method, :params => params}.to_json
      respdata = RestClient.post @url.to_s, postdata
      resp = JSON.parse respdata
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
