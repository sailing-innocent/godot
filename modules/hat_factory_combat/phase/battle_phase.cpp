#include "battle_phase.h"

#include "state/battle_state.h"

#include "core/object/class_db.h"

void BattlePhase::_bind_methods() {
	ClassDB::bind_method(D_METHOD("can_enter", "state"), &BattlePhase::can_enter);
	ClassDB::bind_method(D_METHOD("enter", "state"), &BattlePhase::enter);
	ClassDB::bind_method(D_METHOD("tick", "state", "delta"), &BattlePhase::tick);
	ClassDB::bind_method(D_METHOD("exit", "state"), &BattlePhase::exit);
}

bool BattlePhase::can_enter(const Ref<BattleState> &p_state) const {
	return true;
}

Ref<BattleState> BattlePhase::enter(const Ref<BattleState> &p_state) const {
	return p_state;
}

Ref<BattleState> BattlePhase::tick(const Ref<BattleState> &p_state, double p_delta) const {
	return p_state;
}

Ref<BattleState> BattlePhase::exit(const Ref<BattleState> &p_state) const {
	return p_state;
}
