#pragma once
#include "alias.h"
#include "asset.h"
#include "profile.h"
#include <memory>
#include <type_traits>
#include <unordered_map>

namespace gem {

enum class EventQueue : u32 {
  engine,
  game,
};

enum class EngineEvent : u32 {
  invalid = 0,
  asset_loaded,

};

enum class GameEvent : u32 {
  invalid = 0,
};

struct AEventData {};

#define EVENT_DATA_IMPL(NAME, QUEUE, INDEX)                                    \
  NAME(){};                                                                    \
  static constexpr u32 get_queue() { return static_cast<u32>(QUEUE); }         \
  static constexpr u32 get_index() { return static_cast<u32>(INDEX); }

// sample event data
struct AssetLoadedData : AEventData {
  AssetHandle m_handle_loaded = {};

  EVENT_DATA_IMPL(AssetLoadedData, EventQueue::engine,
                  EngineEvent::asset_loaded)
  GEM_IMPL_ALLOC(AssetLoadedData)
};

// helper struct to quickly find subscribed events
struct EventComparator {
  u32 m_queue;
  u32 m_index;

  EventComparator(const EventComparator &) = default;
  EventComparator &operator=(const EventComparator &) = default;

  bool operator==(EventComparator const &rhs) const {
    ZoneScoped;
    return m_queue == rhs.m_queue && m_index == rhs.m_index;
  }

  bool operator<(const EventComparator &o) const {
    return m_index < o.m_index && m_queue < o.m_queue;
  };

  GEM_IMPL_ALLOC(EventComparator)
};
} // namespace gem

/* required to hash a container */
template <> struct std::hash<gem::EventComparator> {
  std::size_t operator()(const gem::EventComparator &ah) const {
    ZoneScoped;
    return std::hash<u32>()(ah.m_queue) ^ std::hash<u32>()(ah.m_index);
  }
};

namespace gem {

class EventHandler {
protected:
  std::unordered_map<EventComparator, std::vector<void *>> p_subscriptions;

public:
  template <typename _EventData>
  void add_subscription(void (*callback)(_EventData)) {
    ZoneScoped;
    static_assert(std::is_base_of<AEventData, _EventData>(),
                  "_EventData does not inherit from a_event_data");
    static_assert(std::is_default_constructible<_EventData>(),
                  "_EventData is not default constructible");

    _EventData prototype{};
    EventComparator ec{static_cast<u32>(prototype.get_queue()),
                        static_cast<u32>(prototype.get_index())};
    if (p_subscriptions.find(ec) == p_subscriptions.end()) {
      p_subscriptions.emplace(ec, std::vector<void *>());
    }

    p_subscriptions[ec].emplace_back((void *)(callback));
  }

  template <typename _EventData>
  void remove_subscription(void (*callback)(_EventData)) {
    ZoneScoped;
    static_assert(std::is_base_of<AEventData, _EventData>(),
                  "_EventData does not inherit from a_event_data");
    static_assert(std::is_default_constructible<_EventData>(),
                  "_EventData is not default constructible");

    _EventData prototype{};
    EventComparator ec{static_cast<u32>(prototype.get_queue()),
                        static_cast<u32>(prototype.get_index())};
    if (p_subscriptions.find(ec) == p_subscriptions.end()) {
      return;
    }

    void *callback_addr = (void *)(callback);

    int index = -1;

    for (int i = 0; i < p_subscriptions[ec].size(); i++) {
      if (p_subscriptions[ec][i] == callback_addr) {
        index == i;
      }
    }

    if (index >= 0) {
      p_subscriptions[ec].erase(p_subscriptions[ec].begin() + index);
    }
  }

  template <typename _EventData> void invoke(_EventData data) {
    ZoneScoped;
    static_assert(std::is_base_of<AEventData, _EventData>(),
                  "_EventData does not inherit from a_event_data");
    static_assert(std::is_default_constructible<_EventData>(),
                  "_EventData is not default constructible");
    _EventData prototype{};
    EventComparator ec{static_cast<u32>(prototype.get_queue()),
                        static_cast<u32>(prototype.get_index())};
    if (p_subscriptions.find(ec) == p_subscriptions.end()) {
      return;
    }

    for (void *subscription : p_subscriptions[ec]) {
      void (*callback)(_EventData) = (void (*)(_EventData))(subscription);
      callback(data);
    }
  }

  GEM_IMPL_ALLOC(EventHandler)
};
} // namespace gem