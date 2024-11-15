#include "sha1.h"
#include <cassert>

static std::array<uint8_t, 64> PADDING = {
    0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static uint32_t __attribute__((noinline)) rol32(uint32_t x, uint32_t n) {
  return (x << n) ^ (x << (32 - n));
}

Sha1::Sha1() {}

Sha1::Sha1(std::span<uint8_t> data) { update(data); }

void Sha1::update(std::span<uint8_t> data) {
  std::array<uint32_t, 16> values;
  unsigned int offset = size % 64;
  size += data.size();

  for (auto c : data) {
    buf[offset++] = c;

    if (offset % 64 == 0) {
      for (size_t j = 0; j < 16; ++j) {
        values[j] = (uint32_t)(buf[(j * 4) + 0]) << 24 |
                    (uint32_t)(buf[(j * 4) + 1]) << 16 |
                    (uint32_t)(buf[(j * 4) + 2]) << 8 |
                    (uint32_t)(buf[(j * 4 + 3)]);
      }
      update_values(values);
      offset = 0;
    }
  }
}

void Sha1::update_values(const std::array<uint32_t, 16> &values) {
  std::array<uint32_t, 80> w;
  for (size_t i = 0; i < 16; i++) {
    w[i] = values[i];
  }
  for (size_t i = 16; i < 80; i++) {
    w[i] = rol32(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
  }

  uint32_t a = state[0];
  uint32_t b = state[1];
  uint32_t c = state[2];
  uint32_t d = state[3];
  uint32_t e = state[4];

  for (size_t i = 0; i < 80; i++) {
    uint32_t f;
    uint32_t k;
    if (i < 20) {
      f = (b & c) | ((~b) & d);
      k = 0x5A827999;
    } else if (i < 40) {
      f = b ^ c ^ d;
      k = 0x6ED9EBA1;
    } else if (i < 60) {
      f = (b & c) | (b & d) | (c & d);
      k = 0x8F1BBCDC;
    } else {
      f = b ^ c ^ d;
      k = 0xCA62C1D6;
    }

    uint32_t temp = rol32(a, 5) + f + e + k + w[i];
    e = d;
    d = c;
    c = rol32(b, 30);
    b = a;
    a = temp;
  }
  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;
  state[4] += e;
}

std::array<uint32_t, 5> Sha1::digest_values() && {
  std::array<uint32_t, 16> values;
  size_t message_size = size;
  size_t offset = size % 64;
  size_t padding_length = offset < 56 ? 56 - offset : (56 + 64) - offset;

  update(std::span<uint8_t>(PADDING).subspan(0, padding_length));
  ;

  assert(size % 64 == 56);
  for (unsigned int j = 0; j < 14; ++j) {
    values[j] = (uint32_t)(buf[(j * 4) + 0]) << 24 |
                (uint32_t)(buf[(j * 4) + 1]) << 16 |
                (uint32_t)(buf[(j * 4) + 2]) << 8 |
                (uint32_t)(buf[(j * 4) + 3]);
  }
  values[14] = (uint32_t)((message_size * 8) >> 32);
  values[15] = (uint32_t)(message_size * 8);

  update_values(values);
  return state;
}

std::array<uint8_t, 20> Sha1::digest() && {
  auto state = std::move(*this).digest_values();

  std::array<uint8_t, 20> res;

  for (size_t i = 0; i < 5; i++) {
    res[i * 4 + 0] = ((uint8_t)(state[i] >> 24)) & 0xff;
    res[i * 4 + 1] = ((uint8_t)(state[i] >> 16)) & 0xff;
    res[i * 4 + 2] = ((uint8_t)(state[i] >> 8)) & 0xff;
    res[i * 4 + 3] = ((uint8_t)(state[i] >> 0)) & 0xff;
  }

  return res;
}
