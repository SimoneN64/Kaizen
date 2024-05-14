#pragma once
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

static inline nlohmann::json JSONOpenOrCreate(const std::string& path) {
  auto fileExists = fs::exists(path);
  
  if (fileExists) {
    auto file = std::fstream(path, std::fstream::in | std::fstream::out);
    auto json = nlohmann::json::parse(file);
    file.close();
    return json;
  } else {
    auto file = std::fstream(path, std::fstream::in | std::fstream::out | std::fstream::trunc);
    nlohmann::json json;
    json["audio"]["volumeL"] = 0.5;
    json["audio"]["volumeR"] = 0.5;
    json["audio"]["lock"] = true;
    json["cpu"]["type"] = "interpreter";
    json["input"] = {
      {"A", "X"},
      {"B", "C"},
      {"Z", "Z"},
      {"Start", "Return"},
      {"L", "A"},
      {"R", "S"},
      {"Dpad Up", ""},
      {"Dpad Down", ""},
      {"Dpad Left", ""},
      {"Dpad Right", ""},
      {"C Up", "I"},
      {"C Down", "K"},
      {"C Left", "J"},
      {"C Right", "L"},
      {"Analog Up", "Up"},
      {"Analog Down", "Down"},
      {"Analog Left", "Left"},
      {"Analog Right", "Right"},
    };

    file << json;
    file.close();

    return json;
  }
}

template <typename T>
static inline void JSONSetField(nlohmann::json& json, const std::string& field1, const std::string& field2, const T& value) {
  json[field1][field2] = value;
}

template <typename T>
static inline T JSONGetField(nlohmann::json& json, const std::string& field1, const std::string& field2) {
  return json[field1][field2].get<T>();
}