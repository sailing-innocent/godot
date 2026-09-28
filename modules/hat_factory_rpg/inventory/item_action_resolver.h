#ifndef HAT_FACTORY_RPG_ITEM_ACTION_RESOLVER_H
#define HAT_FACTORY_RPG_ITEM_ACTION_RESOLVER_H

#include "core/object/ref_counted.h"

class ActionResult;
class BattleAction;
class BattleState;

/**
 * Resolves BattleAction::ITEM actions against a BattleState.
 *
 * Lives in the RPG module (which depends on the combat module) so that the
 * combat module itself never references item types. BattleEngine dispatches
 * ITEM actions to this class via ClassDB lookup.
 *
 * The resolver applies the referenced ItemDef's effects to the target entity
 * inside the cloned next state; consuming the item from the Backpack is the
 * caller's responsibility (the Backpack is not part of the battle state).
 */
class ItemActionResolver : public RefCounted {
	GDCLASS(ItemActionResolver, RefCounted)

protected:
	static void _bind_methods();

public:
	Ref<ActionResult> resolve(const Ref<BattleState> &p_state, const Ref<BattleAction> &p_action) const;
};

#endif // HAT_FACTORY_RPG_ITEM_ACTION_RESOLVER_H
