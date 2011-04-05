
require 'uri'
require 'json'
require 'rest_client'

module Bitcoin
  CONFFILE = File.join(ENV['HOME'], '.bitcoin', 'bitcoin.conf')

  def self.conf
    parms = File.read(CONFFILE).split("\n")
    parms.inject(Hash.new) {|h, p| h.merge! Hash[*p.split("=")] }
  end

  def self.rpc_proxy(server = {}, user_agent = nil)
    conf = self.conf rescue Hash.new

    username = server.delete(:username) || conf['rpcuser']
    password = server.delete(:password) || conf['rpcpassword']

    default = {
      :host => 'localhost',
      :port => 8332,
      :path => '/'
    }
    uri = URI::HTTP.build(default.merge(server))

    if username
      uri.user     = URI.escape username, /./
      uri.password = URI.escape password, /./
    end

    RPCProxy.new(uri, user_agent)
  end

  class RPCProxy
    attr_reader :url, :user_agent

    class JSONRPCException < RuntimeError; end

    def initialize(uri, user_agent = nil)
      case uri
      when URI
      when String
        uri = URI.parse(uri)
      else
        raise ArgumentError, "bad URI given"
      end

      @url = uri.to_s
      @user_agent = user_agent
    end

    def method_missing(method, *params)
      postdata = {
        :id => 'jsonrpc',
        :method => method,
        :params => params
      }.to_json

      headers = {}
      headers["User-Agent"] = user_agent if user_agent

      begin
        respdata = RestClient.post(url, postdata, headers)
      rescue Errno::EINTR
        retry
      end

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
