#include "i_combat_agent.h"

#include "core/object/class_db.h"
#include "modules/hat_factory_combat/action/battle_action.h"
#include "modules/hat_factory_combat/event/game_event.h"
#include "modules/hat_factory_combat/state/battle_state.h"

void ICombatAgent::_bind_methods() {
	ClassDB::bind_method(D_METHOD("request_action", "state"), &ICombatAgent::request_action);
	ClassDB::bind_method(D_METHOD("observe", "state", "events"), &ICombatAgent::observe);
	ClassDB::bind_method(D_METHOD("reset"), &ICombatAgent::reset);
	ClassDB::bind_method(D_METHOD("get_agent_id"), &ICombatAgent::get_agent_id);
}

Ref<BattleAction> ICombatAgent::request_action(const Ref<BattleState> &p_state) {
	return Ref<BattleAction>();
}

void ICombatAgent::observe(const Ref<BattleState> &p_state, const TypedArray<GameEvent> &p_events) {
}

void ICombatAgent::reset() {
}

String ICombatAgent::get_agent_id() const {
	return "";
}
