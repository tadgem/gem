#pragma once
#include "cpptrace/cpptrace.hpp"
#include "spdlog/spdlog.h"

#define GEM_ASSERT(_cond_, msg)                                                \
  if (!_cond_) {                                                               \
    spdlog::error("ASSERTION FAILED : {}:{} : {}", __FILE__, __LINE__, msg);   \
    spdlog::error(cpptrace::generate_trace().to_string(true));                 \
    exit(-1);                                                                  \
  }