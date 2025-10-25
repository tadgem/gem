#include "gem/fsm.h"
#include "gem/profile.h"
#include "spdlog/spdlog.h"

namespace gem {

FSM::State::State(int state, std::function<int()> action,
                         std::function<void()> entry,
                         std::function<void()> exit)
    : state_(state), action_(action) {
  ZoneScoped;
  has_entry = false;
  has_exit = false;
  if (entry != NULL) {
    entry_procedure_ = entry;
    has_entry = true;
  }
  if (exit != NULL) {
    exit_procedure_ = exit;
    has_exit = true;
  }
}
FSM::FSM() : current_state_(UINT16_MAX), run_entry_(false) {
  ZoneScoped;
}

void FSM::SetStartingState(int state) {
  ZoneScoped;
  current_state_ = state;
}

void FSM::Update() {
  ZoneScoped;
  int index = -1;
  for (int i = 0; i < states_.size(); i++) {
    if (states_[i].state_ == current_state_) {
      index = i;
      break;
    }
  }
  if (index < 0) {
    previous_state_ = current_state_;
    return;
  }
  if (!run_entry_) {
    if (states_[index].has_entry) {
      states_[index].entry_procedure_();
      run_entry_ = true;
    }
  }
  int trigger = states_[index].action_();
  uint8_t numTransitions = 0;
  while (trigger != kNoTrigger) {
    previous_state_ = current_state_;
    if (!run_entry_) {
      if (states_[index].has_entry) {
        states_[index].entry_procedure_();
        run_entry_ = true;
      }
    }
    for (int i = 0; i < transitions_.size(); i++) {
      if (transitions_[i].trigger == trigger) {
        if (transitions_[i].src_state == current_state_) {
          TransitionState(transitions_[i].dst_state);
          numTransitions++;
          break;
        }
      }
    }
    if (current_state_ != previous_state_) {
      trigger = states_[index].action_();
    }
  }
  previous_state_ = current_state_;
}
void FSM::Trigger(int trigger) {
  ZoneScoped;
  for (int i = 0; i < transitions_.size(); i++) {
    if (transitions_[i].trigger == trigger) {
      if (transitions_[i].src_state == current_state_) {
        TransitionState(transitions_[i].dst_state);
        break;
      }
    }
  }
}
void FSM::AddState(int s, std::function<int()> action) {
  ZoneScoped;
  for (int i = 0; i < states_.size(); i++) {
    if (states_[i].state_ == s) {
      spdlog::error("Already have a state in machine for : {}", s);
      return;
    }
  }
  State stateObj = State(s, action);
  states_.emplace_back(stateObj);
}
void FSM::AddEntry(int state, std::function<void()> entry) {
  ZoneScoped;
  for (int i = 0; i < states_.size(); i++) {
    if (states_[i].state_ == state) {
      states_[i].entry_procedure_ = entry;
      states_[i].has_entry = true;
    }
  }
}
void FSM::AddExit(int state, std::function<void()> exit) {
  ZoneScoped;
  for (int i = 0; i < states_.size(); i++) {
    if (states_[i].state_ == state) {
      states_[i].exit_procedure_ = exit;
      states_[i].has_exit = true;
    }
  }
}
void FSM::AddTrigger(int trigger, int src_state, int dst_state) {
  ZoneScoped;
  StateTransition transition{trigger, src_state, dst_state};
  transitions_.emplace_back(transition);
}

void FSM::TransitionState(int new_state) {
  ZoneScoped;
  if (states_[current_state_].has_exit) {
    states_[current_state_].exit_procedure_();
  }
  current_state_ = new_state;
  run_entry_ = false;
}
} // namespace gem