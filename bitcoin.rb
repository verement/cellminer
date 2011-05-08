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
      uri.user     = URI.escape username, /\W/
      uri.password = URI.escape password, /\W/
    end

    RPCProxy.new(uri, user_agent)
  end

  class RPCProxy
    attr_reader :uri, :user_agent, :timeout

    RETRY_INTERVAL = 30
    LONG_POLL_TIMEOUT = 60 * 60 * 12

    class JSONRPCException < RuntimeError; end

    def initialize(uri, user_agent = nil, timeout = nil)
      case uri
      when URI
      when String
        uri = URI.parse(uri)
      else
        raise ArgumentError, "bad URI given"
      end

      @uri = uri
      @user_agent = user_agent
      @timeout = timeout
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
        respdata = RestClient::Request.execute(method: :post,
                                               url: uri.to_s,
                                               payload: postdata,
                                               headers: headers,
                                               timeout: timeout)
      rescue Errno::EINTR
        retry
      rescue Errno::ETIMEDOUT, Errno::EHOSTUNREACH,
          RestClient::RequestTimeout => err
        warn err
        retry
      rescue RestClient::BadGateway, RestClient::BadRequest => err
        warn "%s; waiting %d seconds" % [err, RETRY_INTERVAL]
        sleep RETRY_INTERVAL
        retry
      end

      resp = JSON.parse(respdata)
      if resp['error']
        raise JSONRPCException, resp['error']
      end

      if block_given? and poll_path = respdata.headers[:x_long_polling]
        yield self.class.new(uri + poll_path, user_agent, LONG_POLL_TIMEOUT)
      end

      resp['result']
    end
  end
end
