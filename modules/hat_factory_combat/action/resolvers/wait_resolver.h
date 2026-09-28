#ifndef WAIT_RESOLVER_H
#define WAIT_RESOLVER_H

#include "core/object/ref_counted.h"

class ActionResult;
class BattleAction;
class BattleState;

class WaitResolver : public RefCounted {
	GDCLASS(WaitResolver, RefCounted)

protected:
	static void _bind_methods();

public:
	Ref<ActionResult> resolve(const Ref<BattleState> &p_state, const Ref<BattleAction> &p_action) const;
};

#endif // WAIT_RESOLVER_H
