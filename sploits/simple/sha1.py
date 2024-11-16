from typing import List, Self, Optional

def rol32(x: int, n: int) -> int:
  x %= 2 ** 32
  n %= 32
  return (x << n) ^ (x << (32 - n))

class Sha1:
  PADDING: List[int] = [
    0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  ]
  size: int
  buf: bytearray


  def __init__(self, data: Optional[bytes]=None):
    self.size = 0
    self.buf = bytearray(64)
    self.state = [
      0x67452301,
      0xEFCDAB89,
      0x98BADCFE,
      0x10325476,
      0xC3D2E1F0,
    ]

    if data is not None:
      self.update(data)

  def update(self, data: bytes):
    values = [None for _ in range(16)]
    offset = self.size % 64
    self.size += len(data)

    for c in data:
      self.buf[offset] = c
      offset += 1
      if offset % 64 == 0:
        for i in range(16):
          values[i] = (self.buf[(i * 4) + 0]) << 24 |\
            (self.buf[(i * 4) + 1]) << 16 |\
            (self.buf[(i * 4) + 2]) << 8 |\
            (self.buf[(i * 4) + 3])
        self.update_values(values)
        offset = 0

  def update_values(self, values: List[int]):
    w = [None for _ in range(80)]
    for i in range(16):
      w[i] = values[i]
    for i in range(16, 80):
      w[i] = rol32(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1)

    a = self.state[0]
    b = self.state[1]
    c = self.state[2]
    d = self.state[3]
    e = self.state[4]
    for i in range(80):
      if i < 20:
        f = (b & c) | ((~b) & d)
        k = 0x5A827999
      elif i < 40:
        f = b ^ c ^ d
        k = 0x6ED9EBA1
      elif i < 60:
        f = (b & c) | (b & d) | (c & d)
        k = 0x8F1BBCDC
      else:
        f = b ^ c ^ d
        k = 0xCA62C1D6

      temp = rol32(a, 5) + f + e + k + w[i]
      e = d
      d = c
      c = rol32(b, 30)
      b = a
      a = temp

      a = a % 2 ** 32
      b = b % 2 ** 32
      c = c % 2 ** 32
      d = d % 2 ** 32
      e = e % 2 ** 32
    self.state[0] += a
    self.state[1] += b
    self.state[2] += c
    self.state[3] += d
    self.state[4] += e
    self.state = [i % 2 ** 32 for i in self.state]

  def digest_values(self) -> List[int]:
    values = [None for _ in range(16)]
    message_size = self.size
    offset = self.size % 64
    padding_length = 56 - offset if offset < 56 else (56 + 64) - offset
    self.update(self.PADDING[0 : padding_length])

    assert self.size % 64 == 56
    for i in range(14):
      values[i] = (self.buf[(i * 4) + 0]) << 24 |\
        (self.buf[(i * 4) + 1]) << 16 |\
        (self.buf[(i * 4) + 2]) << 8 |\
        (self.buf[(i * 4) + 3])
    values[14] = ((message_size * 8) >> 32) %  2 ** 32
    values[15] = (message_size * 8) % 2 ** 32
    self.update_values(values)
    return self.state

  def digest(self) -> bytes:
    state = self.digest_values()
    res = bytearray(20)

    for i in range(5):
      res[i * 4 + 0] = ((state[i] >> 24)) & 0xff
      res[i * 4 + 1] = ((state[i] >> 16)) & 0xff
      res[i * 4 + 2] = ((state[i] >> 8)) & 0xff
      res[i * 4 + 3] = ((state[i] >> 0)) & 0xff

    return bytes(res)
