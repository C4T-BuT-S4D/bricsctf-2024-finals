#include <array>
#include <cstdint>
#include <span>

const size_t BLOCK_SIZE = 64;

class Sha1 {
  std::array<uint32_t, 5> state = {
      0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0,
  };
  std::array<uint8_t, BLOCK_SIZE> buf;
  size_t size = 0;

  void update_values(const std::array<uint32_t, 16> &values);

public:
  Sha1();
  Sha1(const std::span<uint8_t> data);

  void update(const std::span<uint8_t> data);

  std::array<uint32_t, 5> digest_values() &&;
  std::array<uint8_t, 20> digest() &&;
};
