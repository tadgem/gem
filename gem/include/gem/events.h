#pragma once
#include "alias.h"
#include "asset.h"
#include "profile.h"
#include <memory>
#include <type_traits>
#include <unordered_map>

namespace gem {

enum class EventQueue : u32 {
  kEngine,
  kGame,
};

enum class EngineEvent : u32 {
  kInvalid = 0,
  kAssetLoaded,

};

enum class GameEvent : u32 {
  kInvalid = 0,
};

struct AEventData {};

#define EVENT_DATA_IMPL(NAME, QUEUE, INDEX)                                    \
  NAME(){};                                                                    \
  static constexpr u32 get_queue() { return static_cast<u32>(QUEUE); }         \
  static constexpr u32 get_index() { return static_cast<u32>(INDEX); }

// sample event data
struct AssetLoadedData : AEventData {
  AssetHandle handle_loaded = {};

  EVENT_DATA_IMPL(AssetLoadedData, EventQueue::kEngine,
                  EngineEvent::kAssetLoaded)
  GEM_IMPL_ALLOC(AssetLoadedData)
};

// helper struct to quickly find subscribed events
struct EventComparator {
  u32 queue;
  u32 index;

  EventComparator(const EventComparator &) = default;
  EventComparator &operator=(const EventComparator &) = default;

  bool operator==(EventComparator const &rhs) const {
    ZoneScoped;
    return queue == rhs.queue && index == rhs.index;
  }

  bool operator<(const EventComparator &o) const {
    return index < o.index && queue < o.queue;
  };

  GEM_IMPL_ALLOC(EventComparator)
};
} // namespace gem

/* required to hash a container */
template <> struct std::hash<gem::EventComparator> {
  std::size_t operator()(const gem::EventComparator &ah) const {
    ZoneScoped;
    return std::hash<u32>()(ah.queue) ^ std::hash<u32>()(ah.index);
  }
};

namespace gem {

class EventHandler {
protected:
  std::unordered_map<EventComparator, std::vector<void *>> subscriptions_;

public:
  template <typename _EventData>
  void AddSubscription(void (*callback)(_EventData)) {
    ZoneScoped;
    static_assert(std::is_base_of<AEventData, _EventData>(),
                  "_EventData does not inherit from a_event_data");
    static_assert(std::is_default_constructible<_EventData>(),
                  "_EventData is not default constructible");

    _EventData prototype{};
    EventComparator ec{static_cast<u32>(prototype.get_queue()),
                        static_cast<u32>(prototype.get_index())};
    if (subscriptions_.find(ec) == subscriptions_.end()) {
      subscriptions_.emplace(ec, std::vector<void *>());
    }

    subscriptions_[ec].emplace_back((void *)(callback));
  }

  template <typename _EventData>
  void RemoveSubscription(void (*callback)(_EventData)) {
    ZoneScoped;
    static_assert(std::is_base_of<AEventData, _EventData>(),
                  "_EventData does not inherit from a_event_data");
    static_assert(std::is_default_constructible<_EventData>(),
                  "_EventData is not default constructible");

    _EventData prototype{};
    EventComparator ec{static_cast<u32>(prototype.get_queue()),
                        static_cast<u32>(prototype.get_index())};
    if (subscriptions_.find(ec) == subscriptions_.end()) {
      return;
    }

    void *callback_addr = (void *)(callback);

    int index = -1;

    for (int i = 0; i < subscriptions_[ec].size(); i++) {
      if (subscriptions_[ec][i] == callback_addr) {
        index == i;
      }
    }

    if (index >= 0) {
      subscriptions_[ec].erase(subscriptions_[ec].begin() + index);
    }
  }

  template <typename _EventData> void Fire(_EventData data) {
    ZoneScoped;
    static_assert(std::is_base_of<AEventData, _EventData>(),
                  "_EventData does not inherit from a_event_data");
    static_assert(std::is_default_constructible<_EventData>(),
                  "_EventData is not default constructible");
    _EventData prototype{};
    EventComparator ec{static_cast<u32>(prototype.get_queue()),
                        static_cast<u32>(prototype.get_index())};
    if (subscriptions_.find(ec) == subscriptions_.end()) {
      return;
    }

    for (void *subscription : subscriptions_[ec]) {
      void (*callback)(_EventData) = (void (*)(_EventData))(subscription);
      callback(data);
    }
  }

  GEM_IMPL_ALLOC(EventHandler)
};
} // namespace gem