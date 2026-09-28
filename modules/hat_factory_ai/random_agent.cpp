#include "random_agent.h"

#include "core/object/class_db.h"
#include "modules/hat_factory_combat/action/battle_action.h"
#include "modules/hat_factory_combat/engine/battle_engine.h"
#include "modules/hat_factory_combat/state/battle_state.h"

void RandomAgent::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_agent_id", "agent_id"), &RandomAgent::set_agent_id);
}

Ref<BattleAction> RandomAgent::request_action(const Ref<BattleState> &p_state) {
	if (p_state.is_null()) {
		return Ref<BattleAction>();
	}
	Ref<BattleEngine> engine;
	engine.instantiate();
	TypedArray<BattleAction> actions = engine->get_legal_actions(p_state, p_state->get_current_actor());
	if (actions.size() == 0) {
		return BattleAction::end_turn(p_state->get_current_actor());
	}
	int idx = p_state->next_random_int(0, actions.size());
	return actions[idx];
}
