#pragma once
#include "gem/dbg_memory.h"
#include "gem/profile.h"
#include <functional>
#include <vector>

#include <vector>
namespace gem {


class FSM {
public:
  const unsigned char MAX_TRANSITIONS_PER_TICK = 8;

  GEM_IMPL_ALLOC(FSM)

  class State {
  public:
    State(int state, std::function<int()> action,
          std::function<void()> entry = NULL,
          std::function<void()> exit = NULL);
    bool m_has_entry;
    bool m_has_exit;
    GEM_IMPL_ALLOC(FSM::State)

  protected:
    std::function<void()> m_entry_procedure;
    std::function<void()> m_exit_procedure;
    std::function<int()> m_action;
    const int m_state;
    friend class FSM;
  };
  struct StateTransition {
    int m_trigger;
    int m_src_state;
    int m_dst_state;

    GEM_IMPL_ALLOC(FSM::StateTransition)
  };

public:
  FSM();

  void SetStartingState(int state);
  template <typename _Enum> void SetStartingState(_Enum state) {
    ZoneScoped;
    SetStartingState(static_cast<int>(state));
  }

  void Update();
  void AddState(int state, std::function<int()> action);
  template <typename _Enum>
  void AddState(_Enum state, std::function<int()> action) {
    ZoneScoped;
    AddState(static_cast<int>(state), action);
  }

  void AddEntry(int state, std::function<void()> entry);
  template <typename _Enum>
  void AddEntry(_Enum state, std::function<void()> action) {
    ZoneScoped;
    AddEntry(static_cast<int>(state), action);
  }

  void AddExit(int state, std::function<void()> exit);
  template <typename _Enum>
  void AddExit(_Enum state, std::function<void()> action) {
    ZoneScoped;
    AddExit(static_cast<int>(state), action);
  }

  void AddTrigger(int trigger, int src_state, int dst_state);
  template <typename _TriggerEnum, typename _StateEnum>
  void AddTrigger(_TriggerEnum trigger, _StateEnum src_state,
                   _StateEnum dst_state) {
    ZoneScoped;
    AddTrigger(static_cast<int>(trigger), static_cast<int>(src_state),
                static_cast<int>(dst_state));
  }

  void Trigger(int trigger);
  template <typename _Enum> void Trigger(_Enum trigger) {
    ZoneScoped;
    Trigger(static_cast<int>(trigger));
  }

  // arbitrary, can probably be done in a better way but we are in trouble if we
  // have an FSM with more than 65000 states.
  inline static const int NO_TRIGGER = 65536;

protected:
  int p_previous_state;
  int p_current_state;
  bool p_run_entry;

  void TransitionState(int new_state);

  std::vector<State> p_states;
  std::vector<StateTransition> p_transitions;
};

using fsm = FSM;
} // namespace gem