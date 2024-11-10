#include "app.h"
#include "util.h"
#include <crow/common.h>
#include <format>
#include <stdexcept>

crow::response Simple::get_note_route(const crow::request &req,
                                      std::string note_id) {
  std::string user_id;
  try {
    user_id = get_user_id_from_req(req);
  } catch (const std::runtime_error &e) {
    return fail(crow::status::UNAUTHORIZED, e.what());
  }

  auto tx = cp::tx(pool);
  auto &work = tx.get();
  auto q = work.query<std::string, std::string>(std::format(
      "select user_id, text from notes where id = {}", work.quote(note_id)));

  if (q.begin() == q.end()) {
    return fail(crow::status::NOT_FOUND, "not does not exist");
  }

  auto [note_user_id, note_text] = *q.begin();

  if (note_user_id != user_id) {
    return fail(crow::status::UNAUTHORIZED,
                "you do not have access to this note");
  }

  crow::json::wvalue resp;
  resp["text"] = note_text;
  return success(crow::status::OK, std::move(resp));
}

crow::response Simple::put_note_route(const crow::request &req) {
  std::string user_id;
  try {
    user_id = get_user_id_from_req(req);
  } catch (const std::runtime_error &e) {
    return fail(crow::status::UNAUTHORIZED, e.what());
  }

  auto json = crow::json::load(req.body);
  if (!json || !json.has("text") ||
      json["text"].t() != crow::json::type::String) {
    return fail(crow::status::BAD_REQUEST, "invalid json");
  }

  auto note_text = std::string(json["text"].s());
  auto tx = cp::tx(pool);
  auto &work = tx.get();
  auto note_id = random_id();

  work.exec_params("insert into notes (id, user_id, text) values ($1, $2, $3)",
                   note_id, user_id, note_text);

  work.commit();

  crow::json::wvalue resp;
  resp["id"] = note_id;
  return success(crow::status::OK, std::move(resp));
}

crow::response Simple::list_notes_route(const crow::request &req) {
  std::string user_id;
  try {
    user_id = get_user_id_from_req(req);
  } catch (const std::runtime_error &e) {
    return fail(crow::status::UNAUTHORIZED, e.what());
  }

  auto tx = cp::tx(pool);
  auto &work = tx.get();
  crow::json::wvalue::list resp;

  for (auto [note_id] : work.query<std::string>(
           std::format("select id from notes where user_id = '{}'", user_id))) {
    resp.push_back(note_id);
  }

  return success(crow::status::OK, std::move(resp));
}
