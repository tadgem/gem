#pragma once
#include <iostream>
#define AASSERT(_cond_, msg)                                                   \
  if (!_cond_) {                                                               \
    std::cerr << "Assertion Failed : " << __FILE__ << " : " << __LINE__        \
              << " : " << msg;                                                 \
    exit(-1);                                                                  \
  }