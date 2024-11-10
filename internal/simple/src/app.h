#pragma once
#include "pqxx-connection-pool.hpp"
#include <crow/http_request.h>
#include <crow/http_response.h>
#include <cstdint>
#include <string_view>
#include <vector>

class Simple {
  std::vector<uint8_t> secret_key;
  cp::connection_pool pool;

  std::string get_user_id_from_token(const std::string_view token);
  std::string get_user_id_from_req(const crow::request &req);
  std::string create_token(const std::string_view id);

public:
  Simple(const std::vector<uint8_t> &secret_key,
         const cp::connection_options &connection_options);
  crow::response login_route(const crow::request &req);
  crow::response register_route(const crow::request &req);
  crow::response get_note_route(const crow::request &req, std::string note_id);
  crow::response put_note_route(const crow::request &req);
  crow::response list_notes_route(const crow::request &req);
};
