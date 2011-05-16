# -*- encoding: utf-8 -*-
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

class SHA256
  K = [0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
       0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
       0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
       0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
       0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
       0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
       0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
       0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
       0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
       0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
       0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
       0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
       0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
       0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
       0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
       0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2]

  H0 = [0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19]

  module Operations
    def SHR(n)
      self >> n
    end

    def ROTR(n)
      (self >> n) | (self << (32 - n))
    end
  end

  module Functions
    include Operations

    def Ch(y, z)
      (self & y) ^ (~self & z)
    end

    def Maj(y, z)
      (self & y) ^ (self & z) ^ (y & z)
    end

    def Σ₀
      ROTR(2) ^ ROTR(13) ^ ROTR(22)
    end

    def Σ₁
      ROTR(6) ^ ROTR(11) ^ ROTR(25)
    end

    def σ₀
      ROTR(7) ^ ROTR(18) ^ SHR(3)
    end

    def σ₁
      ROTR(17) ^ ROTR(19) ^ SHR(10)
    end
  end

  def initialize(wrap = UInt32, init = H0)
    @wrap = wrap.dup.class_exec { include Functions }

    @H = init.map {|v| @wrap.new(v) }
    @K =    K.map {|v| @wrap.new(v) }
  end

  def hash_block(m)
    w = m.map {|v| @wrap.new(v) }
    a, b, c, d, e, f, g, h = @H

    round = Proc.new do |t|
      t₁ = h + e.Σ₁ + e.Ch(f, g) + @K[t] + w[t % 16]
      t₂ =     a.Σ₀ + a.Maj(b, c)

      h = g
      g = f
      f = e
      e = d + t₁
      d = c
      c = b
      b = a
      a = t₁ + t₂
    end

    0.upto(15) do |t|
      round.(t)
    end

    16.upto(63) do |t|
      w[t % 16] = w[(t -  2) % 16].σ₁ +
                  w[(t -  7) % 16]    +
                  w[(t - 15) % 16].σ₀ +
                  w[(t - 16) % 16]
      round.(t)
    end

    @H = [a + @H[0], b + @H[1], c + @H[2], d + @H[3],
          e + @H[4], f + @H[5], g + @H[6], h + @H[7]]

    self
  end

  def update(bytes)
    bytes.unpack('N*').each_slice(16) {|b| hash_block(b) }
    self
  end

  def value
    @H.map(&:value)
  end

  def digest
    value.pack('N*')
  end

  def hexdigest
    value.map {|v| "%08x" % v }.join
  end
end

class UInt32
  attr_reader :value

  def initialize(value)
    @value = value & 0xffffffff
  end

  def to_s
    "%08x" % @value
  end

  def +(other)
    self.class.new(@value + other.value)
  end

  def ^(other)
    self.class.new(@value ^ other.value)
  end

  def &(other)
    self.class.new(@value & other.value)
  end

  def |(other)
    self.class.new(@value | other.value)
  end

  def ~@
    self.class.new(~@value)
  end

  def >>(bits)
    self.class.new(@value >> bits)
  end

  def <<(bits)
    self.class.new(@value << bits)
  end
end

# Unit tests
if __FILE__ == $0
  require 'test/unit'

  class TestSHA256 < Test::Unit::TestCase
    def setup
      @sha256 = SHA256.new
    end

    def test_one_block_message
      @sha256.hash_block([0b01100001011000100110001110000000,
                         0, 0, 0, 0, 0, 0, 0,
                         0, 0, 0, 0, 0, 0, 0, 0x18])
      assert_equal(@sha256.value,
                   [0xba7816bf, 0x8f01cfea, 0x414140de, 0x5dae2223,
                    0xb00361a3, 0x96177a9c, 0xb410ff61, 0xf20015ad])
    end

    def test_multi_block_message
      m = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
      padded = m.unpack('N*') + [0x80000000] + [0] * 15 + [0, 0x1c0]

      @sha256.hash_block(padded[0, 16])
      assert_equal(@sha256.digest,
                   [0x85e655d6, 0x417a1795, 0x3363376a, 0x624cde5c,
                    0x76e09589, 0xcac5f811, 0xcc4b32c1, 0xf20e533a].pack('N*'))

      @sha256.hash_block(padded[16, 16])
      assert_equal(@sha256.hexdigest,
                   "248d6a61" "d20638b8" "e5c02693" "0c3e6039"  \
                   "a33ce459" "64ff2167" "f6ecedd4" "19db06c1")
    end

    def test_long_message
      skip "Time-consuming long message test not run (use arg 'long')" unless
        ARGV.include? "long"

      m = "a" * 1_000_000
      padded = m.unpack('N*') << 0x80000000
      padded += [0] * (15 - padded.length % 16) + [m.length * 8]

      assert_equal(padded.length % 16, 0)

      padded.each_slice(16) do |block|
        @sha256.hash_block(block)
      end

      assert_equal(@sha256.value,
                   [0xcdc76e5c, 0x9914fb92, 0x81a1c7e2, 0x84d73e67,
                    0xf1809a48, 0xa497200e, 0x046d39cc, 0xc7112cd0])
    end
  end
end
