#include "gem/dbg_memory.h"
#include "gem/profile.h"

#ifdef ENABLE_MEMORY_TRACKING

debug_memory_tracker::debug_memory_tracker() {
    ZoneScoped;
    if (s_instance != nullptr) {
        // error;
        return;
    }

    s_instance = this;
}

debug_memory_tracker::~debug_memory_tracker() {
    ZoneScoped;
    s_instance = nullptr;
}

#endif