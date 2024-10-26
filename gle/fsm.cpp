#include "fsm.h"
#include "spdlog/spdlog.h"

fsm_lambda::state::state(int state, std::function<int()> action, std::function<void()> entry, std::function<void()> exit)
	: m_state(state), m_action(action)
{
	m_has_entry = false;
	m_has_exit = false;
	if (entry != NULL) {
		m_entry_procedure = entry;
		m_has_entry = true;
	}
	if (exit != NULL) {
		m_exit_procedure = exit;
		m_has_exit = true;
	}
}
fsm_lambda::fsm_lambda() : p_current_state(UINT16_MAX), p_run_entry(false) { 
}

void fsm_lambda::set_starting_state(int state)
{	
	p_current_state = state;
}

void fsm_lambda::update()
{
	
	int index = -1;
	for (int i = 0; i < p_states.size(); i++) {
		if (p_states[i].m_state == p_current_state) {
			index = i;
			break;
		}
	}
	if (index < 0) {
		p_previous_state = p_current_state;
		return;
	}
	if (!p_run_entry) {
		if (p_states[index].m_has_entry) {
			p_states[index].m_entry_procedure();
			p_run_entry = true;
		}
	}
	int		trigger = p_states[index].m_action();
	uint8_t numTransitions = 0;
	while (trigger != NO_TRIGGER) {
		p_previous_state = p_current_state;
		if (!p_run_entry) {
			if (p_states[index].m_has_entry) {
				p_states[index].m_entry_procedure();
				p_run_entry = true;
			}
		}
		for (int i = 0; i < p_transitions.size(); i++) {
			if (p_transitions[i].m_trigger == trigger) {
				if (p_transitions[i].m_src_state == p_current_state) {
					transition_state(p_transitions[i].m_dst_state);
					numTransitions++;
					break;
				}
			}
		}
		if (p_current_state != p_previous_state) {
			trigger = p_states[index].m_action();
		}
	}
	p_previous_state = p_current_state;
}
void fsm_lambda::trigger_int(int trigger)
{
	
	for (int i = 0; i < p_transitions.size(); i++) {
		if (p_transitions[i].m_trigger == trigger) {
			if (p_transitions[i].m_src_state == p_current_state) {
				transition_state(p_transitions[i].m_dst_state);
				break;
			}
		}
	}
}
void fsm_lambda::add_state(int s, std::function<int()> action)
{
		for (int i = 0; i < p_states.size(); i++) {
		if (p_states[i].m_state == s) {
			spdlog::error("Already have a state in machine for : {}", s);
			return;
		}
	}
	state stateObj = state(s, action);
	p_states.emplace_back(stateObj);
}
void fsm_lambda::add_state_entry(int state, std::function<void()> entry)
{
	
	for (int i = 0; i < p_states.size(); i++) {
		if (p_states[i].m_state == state) {
			p_states[i].m_entry_procedure = entry;
			p_states[i].m_has_entry= true;
		}
	}
}
void fsm_lambda::add_state_exit(int state, std::function<void()> exit)
{
	
	for (int i = 0; i < p_states.size(); i++) {
		if (p_states[i].m_state == state) {
			p_states[i].m_exit_procedure= exit;
			p_states[i].m_has_exit = true;
		}
	}
}
void fsm_lambda::add_trigger(int trigger, int src_state, int dst_state)
{
	
	state_transition transition{ trigger, src_state, dst_state };
	p_transitions.emplace_back(transition);
}

void fsm_lambda::transition_state(int new_state)
{
	
	if (p_states[p_current_state].m_has_exit) {
		p_states[p_current_state].m_exit_procedure();
	}
	p_current_state = new_state;
	p_run_entry = false;
}
