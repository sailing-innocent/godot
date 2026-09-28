#ifndef MOVE_RESOLVER_H
#define MOVE_RESOLVER_H

#include "core/object/ref_counted.h"

class ActionResult;
class BattleAction;
class BattleState;

class MoveResolver : public RefCounted {
	GDCLASS(MoveResolver, RefCounted)

protected:
	static void _bind_methods();

public:
	Ref<ActionResult> resolve(const Ref<BattleState> &p_state, const Ref<BattleAction> &p_action) const;
};

#endif // MOVE_RESOLVER_H
