#include "dbg_memory.h"


#ifdef ENABLE_MEMORY_TRACKING

debug_memory_tracker::debug_memory_tracker() {
    if (s_instance != nullptr) {
        // error;
        return;
    }

    s_instance = this;
}

debug_memory_tracker::~debug_memory_tracker() {
    s_instance = nullptr;
}

#endif