#ifndef I_COMBAT_AGENT_H
#define I_COMBAT_AGENT_H

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/variant/typed_array.h"

class BattleState;
class BattleAction;
class GameEvent;

class ICombatAgent : public RefCounted {
	GDCLASS(ICombatAgent, RefCounted)

protected:
	static void _bind_methods();

public:
	virtual Ref<BattleAction> request_action(const Ref<BattleState> &p_state);
	virtual void observe(const Ref<BattleState> &p_state, const TypedArray<GameEvent> &p_events);
	virtual void reset();
	virtual String get_agent_id() const;
};

#endif // I_COMBAT_AGENT_H
