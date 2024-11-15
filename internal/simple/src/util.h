#pragma once
#include <crow/http_response.h>
#include <crow/json.h>

const std::string ALPHABET =
    "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

crow::response success(int code, crow::json::wvalue &&resp);
crow::response fail(int code, crow::json::wvalue &&resp);
std::string random_id(size_t size = 16);
std::string bytes_to_hex(std::span<uint8_t> bytes);
std::vector<uint8_t> hex_to_bytes(std::string_view hex);
