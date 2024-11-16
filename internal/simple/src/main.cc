#include "app.h"
#include "pqxx-connection-pool.hpp"
#include "sha1.h"
#include <crow/app.h>
#include <crow/common.h>
#include <crow/middlewares/cors.h>
#include <sys/random.h>

std::vector<uint8_t> generate_secret_key() {
  std::vector<uint8_t> res(64);
  getrandom(res.data(), res.size(), 0);
  return res;
}

int main() {
  cp::connection_options options = {
      .dbname = "simple",
      .user = "simple",
      .password = "simple",
      .host = "postgres",
      .port = 5432,
      .connections_count = 64,
  };

  Simple simple(generate_secret_key(), options);
  crow::App<crow::CORSHandler> app;

  CROW_ROUTE(app, "/api/register")
      .methods(crow::HTTPMethod::Post)(
          [&](const crow::request &req) { return simple.register_route(req); });

  CROW_ROUTE(app, "/api/login")
      .methods(crow::HTTPMethod::Post)(
          [&](const crow::request &req) { return simple.login_route(req); });
  CROW_ROUTE(app, "/api/note/<string>")
  ([&](const crow::request &req, std::string note_id) {
    return simple.get_note_route(req, note_id);
  });
  CROW_ROUTE(app, "/api/notes")
  ([&](const crow::request &req) { return simple.list_notes_route(req); });
  CROW_ROUTE(app, "/api/note")
      .methods(crow::HTTPMethod::Put)(
          [&](const crow::request &req) { return simple.put_note_route(req); });

  app.port(2112).multithreaded().run();
}
