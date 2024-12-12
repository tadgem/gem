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
  template <typename T> static std::string get_type_name() {
    ZoneScoped;
    return ctti::type_id<T>().name().str();
  }

  template <typename T> static u64 get_type_hash() {
    ZoneScoped;
    return ctti::type_id<T>().hash();
  }

  static u64 get_string_hash(const std::string &str) {

    return ctti::id_from_name(str).hash();
  }
};

struct HashString {
#ifdef TRACK_HASH_STRING_ORIGINALS
  inline static std::unordered_map<u64, std::string> s_hash_string_originals;
#endif
  HashString() { m_value = 0; }

  HashString(const std::string &input)
      : m_value(HashUtils::get_string_hash(input)) {
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
    s_hash_string_originals[m_value] = input;
#endif
  }

  HashString(u64 value) : m_value(value) {}

  template <typename T> HashString() : m_value(get_type_hash<T>()) {}

  u64 m_value;

  bool operator==(HashString const &rhs) const {
    return m_value == rhs.m_value;
  }

  bool operator<(const HashString &o) const { return m_value < o.m_value; };

  operator u64() const { return m_value; };

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(HashString, m_value)
};

using hash_string_ge = HashString;
} // namespace gem

template <> struct std::hash<gem::HashString> {
  std::size_t operator()(const gem::HashString &h) const {
    return std::hash<u64>()(h.m_value) ^ std::hash<u64>()(h.m_value);
  }
};