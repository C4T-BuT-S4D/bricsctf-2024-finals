#include "app.h"
#include "util.h"
#include <crow/common.h>
#include <crow/json.h>

crow::response Simple::register_route(const crow::request &req) {
  auto json = crow::json::load(req.body);
  if (!json || !json.has("username") || !json.has("password") ||
      json["username"].t() != crow::json::type::String ||
      json["password"].t() != crow::json::type::String) {
    return fail(crow::status::BAD_REQUEST, "invalid json");
  }

  auto username = std::string(json["username"].s());
  auto password = std::string(json["password"].s());

  auto tx = cp::tx(pool);
  auto &work = tx.get();

  auto q = work.exec_params("select id from users where username=$1", username);

  if (!q.empty()) {
    return fail(crow::status::CONFLICT, "username already exists");
  }

  auto id = random_id();
  work.exec_params(
      "insert into users (id, username, password) values ($1, $2, $3)", id,
      username, password);
  work.commit();

  crow::json::wvalue resp;
  resp["token"] = create_token(id);

  return success(crow::status::OK, std::move(resp));
}

crow::response Simple::login_route(const crow::request &req) {
  auto json = crow::json::load(req.body);
  if (!json || !json.has("username") || !json.has("password") ||
      json["username"].t() != crow::json::type::String ||
      json["password"].t() != crow::json::type::String) {
    return fail(crow::status::BAD_REQUEST, "invalid json");
  }

  auto username = std::string(json["username"].s());
  auto password = std::string(json["password"].s());

  auto tx = cp::tx(pool);
  auto &work = tx.get();

  auto q =
      work.exec_params("select id from users where username=$1 and password=$2",
                       username, password);

  if (q.empty()) {
    return fail(crow::status::CONFLICT, "invalid username or password");
  }

  auto id = q[0][0].get<std::string>().value_or("kek");

  crow::json::wvalue resp;
  resp["token"] = create_token(id);

  return success(crow::status::OK, std::move(resp));
}
