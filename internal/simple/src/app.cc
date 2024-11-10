#include "app.h"
#include "sha1.h"
#include "util.h"
#include <crow/json.h>
#include <cstdint>
#include <iostream>
#include <stdexcept>

Simple::Simple(const std::vector<uint8_t> &secret_key,
               const cp::connection_options &connection_options)
    : secret_key(secret_key), pool(connection_options) {}

std::string Simple::get_user_id_from_req(const crow::request &req) {
  auto &auth_header = req.get_header_value("Authorization");

  size_t space_index = auth_header.find(" ");

  auto bearer_part = auth_header.substr(0, space_index);
  auto token =
      auth_header.substr(space_index + 1, auth_header.size() - space_index - 1);

  if (bearer_part != "Bearer") {
    throw std::runtime_error("invalid authorization header");
  }

  return get_user_id_from_token(token);
}

std::string Simple::get_user_id_from_token(const std::string_view token) {
  size_t dot_index = token.find(".");
  auto payload_bytes = hex_to_bytes(token.substr(0, dot_index));
  auto signature =
      hex_to_bytes(token.substr(dot_index + 1, token.size() - dot_index - 1));

  Sha1 hasher;
  hasher.update(secret_key);
  hasher.update(payload_bytes);

  auto digest = std::move(hasher).digest();
  std::vector<uint8_t> digest_vec(digest.begin(), digest.end());

  if (digest_vec != signature) {
    throw std::runtime_error("invalid signature");
  }

  std::string payload_string(payload_bytes.begin(), payload_bytes.end());
  return crow::json::load(payload_string)["id"].s();
}

std::string Simple::create_token(const std::string_view id) {
  crow::json::wvalue payload_json;
  payload_json["id"] = std::string(id);

  auto payload_string = payload_json.dump();
  std::vector<uint8_t> payload_bytes(payload_string.begin(),
                                     payload_string.end());

  Sha1 hasher;
  hasher.update(secret_key);
  hasher.update(payload_bytes);

  auto digest = std::move(hasher).digest();

  return bytes_to_hex(payload_bytes) + "." + bytes_to_hex(digest);
}
