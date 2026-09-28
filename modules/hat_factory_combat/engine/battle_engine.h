#ifndef BATTLE_ENGINE_H
#define BATTLE_ENGINE_H

#include "core/object/ref_counted.h"
#include "core/string/string_name.h"
#include "core/variant/typed_array.h"

#include "modules/hat_factory_combat/action/battle_action.h"

class ActionResult;
class BattleConfig;
class BattleState;
class QueryAPI;

class BattleEngine : public RefCounted {
	GDCLASS(BattleEngine, RefCounted)

protected:
	static void _bind_methods();

public:
	Ref<BattleState> create_initial_state(const Ref<BattleConfig> &p_config) const;
	Ref<ActionResult> submit(const Ref<BattleState> &p_state, const Ref<BattleAction> &p_action) const;
	TypedArray<BattleAction> get_legal_actions(const Ref<BattleState> &p_state, int p_actor_id) const;
	Ref<ActionResult> check_win_condition(const Ref<BattleState> &p_state) const;
	Ref<BattleState> start_turn(const Ref<BattleState> &p_state) const;
	Ref<QueryAPI> query(const Ref<BattleState> &p_state) const;
};

#endif // BATTLE_ENGINE_H
