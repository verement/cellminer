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
require 'net/http/persistent'

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

    # This is a workaround; URI needs escaped username and password
    if username
      uri.user     = URI.escape username, /[^#{URI::PATTERN::UNRESERVED}]/
      uri.password = URI.escape password, /[^#{URI::PATTERN::UNRESERVED}]/
    end

    RPCProxy.new(uri, user_agent)
  end

  class RPCProxy
    LONG_POLL_TIMEOUT = 60 * 60 * 12

    class JSONRPCError < StandardError
      attr_reader :code

      def initialize(json)
        super json['message']
        @code = json['code']
      end
    end

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

      @session = Net::HTTP::Persistent.new
      @session.read_timeout = timeout if timeout
    end

    def method_missing(method, *params)
      request = Net::HTTP::Post.new(@uri.path)
      request['User-Agent'] = @user_agent if @user_agent

      # This is a workaround; URI needs escaped username and password
      # strings but Net:HTTP requires them unescaped (credentials get
      # base64-encoded anyway)
      request.basic_auth(URI.unescape(@uri.user),
                         URI.unescape(@uri.password)) if @uri.user

      request.content_type = 'application/json'
      request.body = {
        id: 'jsonrpc',
        method: method,
        params: params
      }.to_json

      begin
        response = @session.request(@uri, request)
        response.value  # raises error on all non-success responses
      rescue Errno::EINTR
        retry
      end

      json = JSON.parse(response.body)
      raise JSONRPCError.new(json['error']) if json['error']

      if block_given? and poll_path = response['X-Long-Polling']
        yield self.class.new(@uri + poll_path, @user_agent, LONG_POLL_TIMEOUT)
      end

      json['result']
    end
  end
end
