#include "gem/dbg_memory.h"
#include "gem/profile.h"

#ifdef GEM_ENABLE_MEMORY_TRACKING
namespace gem {

DebugMemoryTracker::DebugMemoryTracker() {
  ZoneScoped;
  if (kInstance != nullptr) {
    // error;
    return;
  }

  kInstance = this;
}

DebugMemoryTracker::~DebugMemoryTracker() {
  ZoneScoped;
  kInstance = nullptr;
}
} // namespace gem
#endif