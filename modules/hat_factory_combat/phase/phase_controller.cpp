#include "phase_controller.h"

#include "battle_phase.h"
#include "state/battle_state.h"

#include "core/object/class_db.h"

void PhaseController::_bind_methods() {
	ClassDB::bind_method(D_METHOD("register_phase", "name", "phase"), &PhaseController::register_phase);
	ClassDB::bind_method(D_METHOD("transition_to", "state", "phase_name"), &PhaseController::transition_to);
	ClassDB::bind_method(D_METHOD("tick", "state", "delta"), &PhaseController::tick);
	ClassDB::bind_method(D_METHOD("get_current_phase"), &PhaseController::get_current_phase);
}

void PhaseController::register_phase(const StringName &p_name, const Ref<BattlePhase> &p_phase) {
	if (p_phase.is_null()) {
		phases.erase(p_name);
	} else {
		phases[p_name] = p_phase;
	}
}

Ref<BattleState> PhaseController::transition_to(const Ref<BattleState> &p_state, const StringName &p_phase_name) {
	if (p_state.is_null()) {
		return p_state;
	}
	Ref<BattlePhase> current;
	if (phases.has(current_phase_name)) {
		current = phases[current_phase_name];
	}
	Ref<BattleState> next = p_state->clone();
	if (current.is_valid()) {
		next = current->exit(next);
	}
	Ref<BattlePhase> target;
	if (phases.has(p_phase_name)) {
		target = phases[p_phase_name];
	}
	if (target.is_null()) {
		return next;
	}
	if (target->can_enter(next)) {
		next = target->enter(next);
		next->set_phase(p_phase_name);
		current_phase_name = p_phase_name;
	}
	return next;
}

Ref<BattleState> PhaseController::tick(const Ref<BattleState> &p_state, double p_delta) {
	if (p_state.is_null()) {
		return p_state;
	}
	Ref<BattlePhase> current;
	if (phases.has(current_phase_name)) {
		current = phases[current_phase_name];
	}
	if (current.is_null()) {
		return p_state;
	}
	Ref<BattleState> next = p_state->clone();
	return current->tick(next, p_delta);
}
