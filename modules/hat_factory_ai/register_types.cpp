#include "register_types.h"

#include "core/object/class_db.h"

#include "i_combat_agent.h"
#include "random_agent.h"
#include "behavior_tree_agent.h"
#include "rl_bridge.h"

void initialize_hat_factory_ai_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		GDREGISTER_CLASS(ICombatAgent);
		GDREGISTER_CLASS(RandomAgent);
		GDREGISTER_CLASS(BehaviorTreeAgent);
		GDREGISTER_CLASS(RLBridge);
	}
}

void uninitialize_hat_factory_ai_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
}
