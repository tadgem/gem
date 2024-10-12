#pragma once
#include "alias.h"
#include "ctti/type_id.hpp"

#define TRACK_HASH_STRING_ORIGINALS

#ifdef TRACK_HASH_STRING_ORIGINALS
#include <unordered_map>
#endif

template <typename T>
std::string get_type_name() { return ctti::type_id<T>().name(); }

template <typename T>
u64 get_type_hash() { return ctti::type_id<T>().hash(); }

u64 get_string_hash(const std::string& str) {
    u64 ret = 0;
    for (auto& c : str)
    {
        ret ^= 2305 * c;
    }
    return ret;
}

struct hash_string
{
#ifdef TRACK_HASH_STRING_ORIGINALS
    inline static std::unordered_map<u64, std::string> s_hash_string_originals;
#endif
    hash_string() { m_value = 0; }

    hash_string(const std::string& input) : m_value(get_string_hash(input)) {
#ifdef TRACK_HASH_STRING_ORIGINALS
        s_hash_string_originals[m_value] = input;
#endif
    }

    hash_string(u64 value) : m_value(value) {}

    template <typename T>
    hash_string() : m_value(get_type_hash<T>()) {}

    u64 m_value;

    bool operator==(hash_string const& rhs) const
    {
        return m_value == rhs.m_value;
    }

    bool operator<(const hash_string& o) const { return m_value < o.m_value; };

    operator u64() const { return m_value; };
};

template <>
struct std::hash<hash_string>
{
    std::size_t operator()(const hash_string& h) const
    {
        return std::hash<u64>()(h.m_value) ^
            std::hash<u64>()(h.m_value);
    }
};