#include "util.h"
#include <random>
#include <stdexcept>
#include <string_view>

crow::response success(int code, crow::json::wvalue &&resp) {
  crow::json::wvalue res;
  res["message"] = std::move(resp);
  res["status"] = "success";
  return crow::response(code, res);
}

crow::response fail(int code, crow::json::wvalue &&resp) {
  crow::json::wvalue res;
  res["message"] = std::move(resp);
  res["status"] = "fail";
  return crow::response(code, res);
}

std::string random_id(size_t size) {
  std::random_device rd;
  std::seed_seq seed{rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd()};
  std::mt19937 g(seed);
  std::uniform_int_distribution<int> dist(0, ALPHABET.size() - 1);

  std::string res = "";

  for (size_t i = 0; i < size; i++) {
    res += ALPHABET[dist(g)];
  }

  return res;
}

std::string bytes_to_hex(std::span<uint8_t> bytes) {
  std::string res;
  for (auto b : bytes) {
    res += std::format("{:02x}", b);
  }
  return res;
}

static uint8_t hexdigit_to_int(char digit) {
  if ('0' <= digit && digit <= '9') {
    return digit - '0';
  }
  if ('a' <= digit && digit <= 'f') {
    return digit - 'a' + 10;
  }
  if ('A' <= digit && digit <= 'F') {
    return digit - 'A' + 10;
  }

  throw std::runtime_error("non hexadecimal digit found");
}

std::vector<uint8_t> hex_to_bytes(std::string_view hex) {
  if (hex.size() % 2 != 0) {
    throw std::runtime_error("odd length string");
  }

  std::vector<uint8_t> res;

  for (size_t i = 0; i < hex.size(); i += 2) {
    res.push_back((hexdigit_to_int(hex[i]) << 4) | hexdigit_to_int(hex[i + 1]));
  }
  return res;
}
