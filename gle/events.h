#pragma once
#include "alias.h"
#include "asset.h"
#include <unordered_map>
#include <memory>
#include <type_traits>

enum class event_queue : u32
{
	engine,
	game,
};

enum class engine_event : u32
{
	invalid = 0,
	asset_loaded,

};

enum class game_event : u32
{
	invalid = 0,
};


struct a_event_data
{
};

#define EVENT_DATA_IMPL(NAME, QUEUE, INDEX)\
NAME() {};\
static constexpr u32 get_queue() {return static_cast<u32>(QUEUE);}\
static constexpr u32 get_index() {return static_cast<u32>(INDEX);}\


// sample event data
struct asset_loaded_data : a_event_data
{
	asset_handle	m_handle_loaded = {};

	EVENT_DATA_IMPL(asset_loaded_data, event_queue::engine, engine_event::asset_loaded)
};

// helper struct to quickly find subscribed events
struct event_comparator
{
	u32	m_queue;
	u32 m_index;

	event_comparator(const event_comparator&) = default;
	event_comparator& operator=(const event_comparator&) = default;

	bool operator==(event_comparator const& rhs) const
	{
		return m_queue == rhs.m_queue && m_index == rhs.m_index;
	}

	bool operator<(const event_comparator& o) const { return m_index < o.m_index && m_queue < o.m_queue; };
};

/* required to hash a container */
template<>
struct std::hash<event_comparator> {
	std::size_t operator()(const event_comparator& ah) const {
		return std::hash<u32>()(ah.m_queue) ^ std::hash<u32>()(ah.m_index);
	}
};


class event_handler
{	
protected:
	std::unordered_map<event_comparator, std::vector<void*>>	p_subscriptions;

public:
	template<typename _EventData>
	void add_subscription(void(*callback) (_EventData))
	{
		static_assert(std::is_base_of<a_event_data, _EventData>(), "_EventData does not inherit from a_event_data");
		static_assert(std::is_default_constructible<_EventData>(), "_EventData is not default constructible");

		_EventData prototype{};
		event_comparator ec{ static_cast<u32>(prototype.get_queue()), static_cast<u32>(prototype.get_index()) };
		if (p_subscriptions.find(ec) == p_subscriptions.end())
		{
			p_subscriptions.emplace(ec, std::vector<void*>());
		}

		p_subscriptions[ec].emplace_back((void*)(callback));
	}

	template<typename _EventData>
	void remove_subscription(void(*callback) (_EventData))
	{
		static_assert(std::is_base_of<a_event_data, _EventData>(), "_EventData does not inherit from a_event_data");
		static_assert(std::is_default_constructible<_EventData>(), "_EventData is not default constructible");

		_EventData prototype{};
		event_comparator ec{ static_cast<u32>(prototype.get_queue()), static_cast<u32>(prototype.get_index()) };
		if (p_subscriptions.find(ec) == p_subscriptions.end())
		{
			return;
		}

		void* callback_addr = (void*)(callback);

		int index = -1;

		for (int i = 0; i < p_subscriptions[ec].size(); i++)
		{
			if (p_subscriptions[ec][i] == callback_addr)
			{
				index == i;
			}
		}

		if (index >= 0)
		{
			p_subscriptions[ec].erase(p_subscriptions[ec].begin() + index);
		}
	}

	template<typename _EventData>
	void invoke(_EventData data)
	{
		static_assert(std::is_base_of<a_event_data, _EventData>(), "_EventData does not inherit from a_event_data");
		static_assert(std::is_default_constructible<_EventData>(), "_EventData is not default constructible");
		_EventData prototype{};
		event_comparator ec{ static_cast<u32>(prototype.get_queue()), static_cast<u32>(prototype.get_index()) };
		if (p_subscriptions.find(ec) == p_subscriptions.end())
		{
			return;
		}

		for (void* subscription : p_subscriptions[ec])
		{
			void(*callback) (_EventData) = (void(*)(_EventData))(subscription);
			callback(data);
		}
	}
};