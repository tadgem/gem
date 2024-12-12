#pragma once
#define GEM_ENABLE_MEMORY_TRACKING
#ifdef GEM_ENABLE_MEMORY_TRACKING

#include "alias.h"
#include <memory>
#include <string>
#include <unordered_map>
#endif

namespace gem {
#ifdef GEM_ENABLE_MEMORY_TRACKING
struct DebugAllocInfo {
  size_t count;
  size_t size;
};

class DebugMemoryTracker {
public:
  DebugMemoryTracker();

  ~DebugMemoryTracker();

  std::unordered_map<std::string, DebugAllocInfo> s_allocation_info;

  inline static u64 s_UntrackedSize = 0;
  inline static DebugMemoryTracker *s_instance = nullptr;
};

#define GEM_IMPL_ALLOC(X)                                                      \
  void *operator new(size_t size) {                                            \
    if (DebugMemoryTracker::s_instance->s_allocation_info.find(#X) ==          \
        DebugMemoryTracker::s_instance->s_allocation_info.end()) {             \
      DebugMemoryTracker::s_instance->s_allocation_info.emplace(             \
          #X, DebugAllocInfo{0, 0});                                               \
    }                                                                          \
    DebugMemoryTracker::s_instance->s_allocation_info[#X].count++;             \
    DebugMemoryTracker::s_instance->s_allocation_info[#X].size += size;      \
    return malloc(size);                                                       \
  }                                                                            \
  void operator delete(void *p) {                                              \
    free(p);                                                                   \
    if (!DebugMemoryTracker::s_instance)                                     \
      return;                                                                  \
    DebugMemoryTracker::s_instance->s_allocation_info[#X].count--;             \
    DebugMemoryTracker::s_instance->s_allocation_info[#X].size -= sizeof(X); \
  }
#else
#define GEM_IMPL_ALLOC(X)
#endif
} // namespace gem