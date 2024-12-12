#include "gem/dbg_memory.h"
#include "gem/profile.h"

#ifdef GEM_ENABLE_MEMORY_TRACKING
namespace gem {

DebugMemoryTracker::DebugMemoryTracker() {
  ZoneScoped;
  if (s_instance != nullptr) {
    // error;
    return;
  }

  s_instance = this;
}

DebugMemoryTracker::~DebugMemoryTracker() {
  ZoneScoped;
  s_instance = nullptr;
}
} // namespace gem
#endif