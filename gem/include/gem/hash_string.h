#pragma once
#include "alias.h"
#include "ctti/type_id.hpp"
#include "gem/profile.h"
#include "json.hpp"
#include "spdlog/spdlog.h"

#define TRACK_HASH_STRING_ORIGINALS
// #define CHECK_FOR_HASH_STRING_COLLISIONS

#ifdef TRACK_HASH_STRING_ORIGINALS
#include <unordered_map>
#endif

namespace gem {

class HashUtils {
public:
  template <typename T> static std::string GetTypeName() {
    ZoneScoped;
    return ctti::type_id<T>().name().str();
  }

  template <typename T> static u64 GetTypeHash() {
    ZoneScoped;
    return ctti::type_id<T>().hash();
  }

  static u64 GetStringHash(const std::string &str) {

    return ctti::id_from_name(str).hash();
  }
};

struct HashString {
#ifdef TRACK_HASH_STRING_ORIGINALS
  inline static std::unordered_map<u64, std::string> s_hash_string_originals;
#endif
  HashString() { hash_value = 0; }

  HashString(const std::string &input)
      : hash_value(HashUtils::GetStringHash(input)) {
    ZoneScoped;
#ifdef TRACK_HASH_STRING_ORIGINALS
#ifdef CHECK_FOR_HASH_STRING_COLLISIONS
    if (s_hash_string_originals.find(m_value) !=
        s_hash_string_originals.end()) {
      if (s_hash_string_originals[m_value] != input) {
        spdlog::error("HASH STRING COLLISION : ORIGINAL : {}, NEW : {}",
                      s_hash_string_originals[m_value], input);
      }
    }
#endif
    s_hash_string_originals[hash_value] = input;
#endif
  }

  HashString(u64 value) : hash_value(value) {}

  template <typename T> HashString() : hash_value(HashUtils::GetTypeHash<T>()) {}

  u64 hash_value;

  bool operator==(HashString const &rhs) const {
    return hash_value == rhs.hash_value;
  }

  bool operator<(const HashString &o) const { return hash_value < o.hash_value; };

  operator u64() const { return hash_value; };

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(HashString, hash_value)
};

using hash_string_ge = HashString;
} // namespace gem

template <> struct std::hash<gem::HashString> {
  std::size_t operator()(const gem::HashString &h) const {
    return std::hash<u64>()(h.hash_value) ^ std::hash<u64>()(h.hash_value);
  }
};