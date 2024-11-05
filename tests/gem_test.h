//
// Created by liam_ on 11/5/2024.
//
#pragma once
#include <string>
#include <unordered_map>
#include <functional>

enum class TEST_RESULT {
  PASS,
  FAIL,
  DNF,
};

static std::string get_string_test_result(TEST_RESULT result)
{
  switch(result){
  case TEST_RESULT::PASS:
    return "PASS";
  case TEST_RESULT::FAIL:
    return "FAIL";
  case TEST_RESULT::DNF:
    return "DNF";
  }

  return "";
}

inline static std::unordered_map<std::string, std::function<TEST_RESULT()>> s_tests = {};

#define BEGIN_TESTS() int main(int argc, char** argv) {
#define TEST(NAME, X) s_tests.emplace(NAME,[]() X);

#define RUN_TESTS() \
for(auto& [name, func] : s_tests)                                                  \
{                   \
    TEST_RESULT result = func();                \
    spdlog::info("Test : {} : Status : {}", name, get_string_test_result(result));                                       \
}}