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
    bool has_entry;
    bool has_exit;
    GEM_IMPL_ALLOC(FSM::State)

  protected:
    std::function<void()> entry_procedure_;
    std::function<void()> exit_procedure_;
    std::function<int()> action_;
    const int state_;
    friend class FSM;
  };
  struct StateTransition {
    int trigger;
    int src_state;
    int dst_state;

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
  inline static const int kNoTrigger = 65536;

protected:
  int previous_state_;
  int current_state_;
  bool run_entry_;

  void TransitionState(int new_state);

  std::vector<State> states_;
  std::vector<StateTransition> transitions_;
};

} // namespace gem