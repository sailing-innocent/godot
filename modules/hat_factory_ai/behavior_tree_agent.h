#ifndef BEHAVIOR_TREE_AGENT_H
#define BEHAVIOR_TREE_AGENT_H

#include "i_combat_agent.h"
#include "core/object/script_language.h"

class BehaviorTreeAgent : public ICombatAgent {
	GDCLASS(BehaviorTreeAgent, ICombatAgent)

	Ref<Script> behavior_tree_script;
	String agent_id = "behavior_tree";

protected:
	static void _bind_methods();

public:
	void set_behavior_tree_script(const Ref<Script> &p_script) { behavior_tree_script = p_script; }
	Ref<Script> get_behavior_tree_script() const { return behavior_tree_script; }

	void set_agent_id(const String &p_id) { agent_id = p_id; }
	String get_agent_id() const override { return agent_id; }

	Ref<BattleAction> request_action(const Ref<BattleState> &p_state) override;
};

#endif // BEHAVIOR_TREE_AGENT_H
