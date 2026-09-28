#ifndef END_TURN_RESOLVER_H
#define END_TURN_RESOLVER_H

#include "core/object/ref_counted.h"

class ActionResult;
class BattleAction;
class BattleState;

class EndTurnResolver : public RefCounted {
	GDCLASS(EndTurnResolver, RefCounted)

protected:
	static void _bind_methods();

public:
	Ref<ActionResult> resolve(const Ref<BattleState> &p_state, const Ref<BattleAction> &p_action) const;
};

#endif // END_TURN_RESOLVER_H
