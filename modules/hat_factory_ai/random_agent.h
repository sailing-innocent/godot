#ifndef RANDOM_AGENT_H
#define RANDOM_AGENT_H

#include "i_combat_agent.h"

class RandomAgent : public ICombatAgent {
	GDCLASS(RandomAgent, ICombatAgent)

	String agent_id = "random";

protected:
	static void _bind_methods();

public:
	void set_agent_id(const String &p_id) { agent_id = p_id; }
	String get_agent_id() const override { return agent_id; }

	Ref<BattleAction> request_action(const Ref<BattleState> &p_state) override;
};

#endif // RANDOM_AGENT_H
