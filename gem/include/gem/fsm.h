#pragma once
#include <functional>
#include <vector>
#include "gem/dbg_memory.h"
#include "gem/profile.h"

#include <vector>
namespace gem {
template <typename _StateEnum> class fsm_t {
public:
  using callback = void (*)();
  using callback_t = void (*)(_StateEnum);

  const unsigned char MAX_TRANSITIONS_PER_TICK = 8;

  GEM_IMPL_ALLOC(fsm_t<_StateEnum>)

  class state {
  public:
    state(_StateEnum state, callback_t action, callback entry = nullptr,
          callback exit = nullptr) {}
    bool m_has_entry;
    bool m_has_exit;

    GEM_IMPL_ALLOC(fsm_t<_StateEnum>::state)


  protected:
    callback m_entry_procedure;
    callback m_exit_procedure;
    callback_t m_action;
    const _StateEnum m_state;
    friend class fsm_t;
  };

  struct state_transition {
    _StateEnum trigger;
    _StateEnum src_state;
    _StateEnum dst_state;

    GEM_IMPL_ALLOC(fsm_t<_StateEnum>::state_transition)

  };
};

class fsm_lambda {
public:
  const unsigned char MAX_TRANSITIONS_PER_TICK = 8;

  GEM_IMPL_ALLOC(fsm_lambda)


  class state {
  public:
    state(int state, std::function<int()> action,
          std::function<void()> entry = NULL,
          std::function<void()> exit = NULL);
    bool m_has_entry;
    bool m_has_exit;
    GEM_IMPL_ALLOC(fsm_lambda::state)


  protected:
    std::function<void()> m_entry_procedure;
    std::function<void()> m_exit_procedure;
    std::function<int()> m_action;
    const int m_state;
    friend class fsm_lambda;
  };
  struct state_transition {
    int m_trigger;
    int m_src_state;
    int m_dst_state;

    GEM_IMPL_ALLOC(fsm_lambda::state_transition)

  };

public:
  fsm_lambda();

  void set_starting_state(int state);
  template <typename _Enum> void set_starting_state(_Enum state) {
    ZoneScoped;
    set_starting_state(static_cast<int>(state));
  }

  void update();
  void add_state(int state, std::function<int()> action);
  template <typename _Enum>
  void add_state(_Enum state, std::function<int()> action) {
    ZoneScoped;
    add_state(static_cast<int>(state), action);
  }

  void add_state_entry(int state, std::function<void()> entry);
  template <typename _Enum>
  void add_state_entry(_Enum state, std::function<void()> action) {
    ZoneScoped;
    add_state_entry(static_cast<int>(state), action);
  }

  void add_state_exit(int state, std::function<void()> exit);
  template <typename _Enum>
  void add_state_exit(_Enum state, std::function<void()> action) {
    ZoneScoped;
    add_state_exit(static_cast<int>(state), action);
  }

  void add_trigger(int trigger, int src_state, int dst_state);
  template <typename _TriggerEnum, typename _StateEnum>
  void add_trigger(_TriggerEnum trigger, _StateEnum src_state,
                   _StateEnum dst_state) {
    ZoneScoped;
    add_trigger(static_cast<int>(trigger), static_cast<int>(src_state),
                static_cast<int>(dst_state));
  }

  void trigger_int(int trigger);
  template <typename _Enum> void trigger(_Enum trigger) {
    ZoneScoped;
    trigger_int(static_cast<int>(trigger));
  }

  // arbitrary, can probably be done in a better way but we are in trouble if we
  // have an FSM with more than 65000 states.
  inline static const int NO_TRIGGER = 65536;

protected:
  int p_previous_state;
  int p_current_state;
  bool p_run_entry;

  void transition_state(int new_state);

  std::vector<state> p_states;
  std::vector<state_transition> p_transitions;
};

using fsm = fsm_lambda;
} // namespace gem