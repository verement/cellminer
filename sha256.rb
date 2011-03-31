
class UInt32
  attr_reader :value

  def self.[](value)
    new(value)
  end

  def initialize(value)
    @value = value & 0xffffffff
  end

  def to_i
    value
  end

  def coerce(other)
    case other
    when Integer
      return self.class.new(other), self
    when Expression
      return other, Expression::Atom::Integer::UInt32.new(value)
    else
      raise TypeError, "#{self.class} can't be coerced into #{other.class}"
    end
  end

  def +(other)
    case other
    when UInt32
      other = other.value
    when Integer
    else
      x, y = other.coerce(self)
      return x + y
    end

    self.class.new(@value + other)
  end

  def ^(other)
    case other
    when UInt32
      other = other.value
    when Integer
    else
      x, y = other.coerce(self)
      return x ^ y
    end

    self.class.new(@value ^ other)
  end

  def &(other)
    case other
    when UInt32
      other = other.value
    when Integer
    else
      x, y = other.coerce(self)
      return x & y
    end

    self.class.new(@value & other)
  end

  def |(other)
    case other
    when UInt32
      other = other.value
    when Integer
    else
      x, y = other.coerce(self)
      return x | y
    end

    self.class.new(@value | other)
  end

  def >>(bits)
    self.class.new(@value >> bits)
  end

  def <<(bits)
    self.class.new(@value << bits)
  end

  def ~
    self.class.new(~@value)
  end
end

class SHA256
  CLASS = UInt32

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

  def self.Ch(x, y, z)
    (x & y) ^ (~x & z)
  end

  def self.Maj(x, y, z)
    (x & y) ^ (x & z) ^ (y & z)
  end

  def self.SHR(n, x)
    x >> n
  end

  def self.ROTR(n, x)
    (x >> n) | (x << (32 - n))
  end

  def self.SIGMA0(x)
    self.ROTR(2, x) ^ self.ROTR(13, x) ^ self.ROTR(22, x)
  end

  def self.SIGMA1(x)
    self.ROTR(6, x) ^ self.ROTR(11, x) ^ self.ROTR(25, x)
  end

  def self.sigma0(x)
    self.ROTR(7, x) ^ self.ROTR(18, x) ^ self.SHR(3, x)
  end

  def self.sigma1(x)
    self.ROTR(17, x) ^ self.ROTR(19, x) ^ self.SHR(10, x)
  end

  def initialize(state = H0)
    @H = state.map {|v| CLASS[v] }
    @length = 0

    @partial = ""
  end

  def hash_block(m)
    m.map! {|v| CLASS[v] }

    a, b, c, d, e, f, g, h = @H
    w = Array.new(16)

    (0..15).each do |t|
      w[t] = m[t]

      t1 = h + self.class.SIGMA1(e) + self.class.Ch(e, f, g) + K[t] + w[t]
      t2 = self.class.SIGMA0(a) + self.class.Maj(a, b, c)
      h = g
      g = f
      f = e
      e = d + t1
      d = c
      c = b
      b = a
      a = t1 + t2
    end

    (16..63).each do |t|
      w[t % 16] = self.class.sigma1(w[(t - 2) % 16]) + w[(t - 7) % 16] +
        self.class.sigma0(w[(t - 15) % 16]) + w[(t - 16) % 16]

      t1 = h + self.class.SIGMA1(e) + self.class.Ch(e, f, g) + K[t] + w[t % 16]
      t2 = self.class.SIGMA0(a) + self.class.Maj(a, b, c)
      h = g
      g = f
      f = e
      e = d + t1
      d = c
      c = b
      b = a
      a = t1 + t2
    end

    @H = [a + @H[0], b + @H[1], c + @H[2], d + @H[3],
          e + @H[4], f + @H[5], g + @H[6], h + @H[7]]
  end

  def update(bytes)
    @length += bytes.length
    @partial += bytes

    while @partial.length >= 64
      hash_block(@partial[0..63].unpack('N16'))
      @partial = @partial[64..-1]
    end

    self
  end

  alias << update

  def finish(bitlen = @length * 8)
    @partial += "\x80"

    if @partial.length > 56
      @partial += "\0" * (64 - @partial.length)
      hash_block(@partial.unpack('N16'))
      @partial = ""
    end

    @partial += "\0" * (56 - @partial.length)
    @partial += [bitlen >> 32, bitlen & 0xffffffff].pack('NN')

    hash_block(@partial.unpack('N16'))

    @partial = nil
    @length.freeze

    self
  end

  def value
    @H
  end

  def digest
    @H.map {|v| v.value }.pack('N*')
  end

  def hexdigest
    @H.map {|v| "%08x" % v.value }.join
  end
end
