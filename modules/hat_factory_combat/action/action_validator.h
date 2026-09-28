#ifndef ACTION_VALIDATOR_H
#define ACTION_VALIDATOR_H

#include "core/object/ref_counted.h"

class ActionResult;
class BattleAction;
class BattleState;

class ActionValidator : public RefCounted {
	GDCLASS(ActionValidator, RefCounted)

protected:
	static void _bind_methods();

public:
	Ref<ActionResult> validate(const Ref<BattleState> &p_state, const Ref<BattleAction> &p_action) const;

	Ref<ActionResult> validate_move(const Ref<BattleState> &p_state, const Ref<BattleAction> &p_action) const;
	Ref<ActionResult> validate_skill(const Ref<BattleState> &p_state, const Ref<BattleAction> &p_action) const;
};

#endif // ACTION_VALIDATOR_H
