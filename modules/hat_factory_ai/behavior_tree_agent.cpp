#include "behavior_tree_agent.h"

#include "core/object/class_db.h"
#include "modules/hat_factory_combat/action/battle_action.h"
#include "modules/hat_factory_combat/state/battle_state.h"

void BehaviorTreeAgent::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_behavior_tree_script", "behavior_tree_script"), &BehaviorTreeAgent::set_behavior_tree_script);
	ClassDB::bind_method(D_METHOD("get_behavior_tree_script"), &BehaviorTreeAgent::get_behavior_tree_script);
	ClassDB::bind_method(D_METHOD("set_agent_id", "agent_id"), &BehaviorTreeAgent::set_agent_id);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "behavior_tree_script", PROPERTY_HINT_RESOURCE_TYPE, "Script"), "set_behavior_tree_script", "get_behavior_tree_script");
}

Ref<BattleAction> BehaviorTreeAgent::request_action(const Ref<BattleState> &p_state) {
	// Stub: returns end_turn. GDScript subclasses or script instances override this.
	if (p_state.is_null()) {
		return Ref<BattleAction>();
	}
	return BattleAction::end_turn(p_state->get_current_actor());
}
