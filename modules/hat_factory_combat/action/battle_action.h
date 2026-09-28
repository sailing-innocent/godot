#ifndef BATTLE_ACTION_H
#define BATTLE_ACTION_H

#include "core/math/vector2i.h"
#include "core/object/ref_counted.h"
#include "core/string/string_name.h"
#include "core/variant/dictionary.h"
#include "core/variant/typed_array.h"

class BattleAction : public RefCounted {
	GDCLASS(BattleAction, RefCounted)

public:
	enum ActionType {
		MOVE = 0,
		SKILL = 1,
		ITEM = 2,
		WAIT = 3,
		END_TURN = 4,
		ESCAPE = 5,
	};

private:
	int action_id = 0;
	int type = MOVE;
	int actor_id = 0;
	Dictionary payload;

protected:
	static void _bind_methods();

public:
	void set_action_id(int p_value) { action_id = p_value; }
	int get_action_id() const { return action_id; }

	void set_type(int p_value) { type = p_value; }
	int get_type() const { return type; }

	void set_actor_id(int p_value) { actor_id = p_value; }
	int get_actor_id() const { return actor_id; }

	void set_payload(const Dictionary &p_value) { payload = p_value; }
	Dictionary get_payload() const { return payload; }

	static Ref<BattleAction> move(int p_actor, const TypedArray<Vector2i> &p_path);
	static Ref<BattleAction> skill(int p_actor, const StringName &p_skill_id, const Vector2i &p_target, int p_target_entity = 0);
	static Ref<BattleAction> item(int p_actor, const StringName &p_item_id, int p_target_entity = 0);
	static Ref<BattleAction> wait(int p_actor);
	static Ref<BattleAction> end_turn(int p_actor);
	static Ref<BattleAction> escape(int p_actor);

	Dictionary to_dict() const;
	static Ref<BattleAction> from_dict(const Dictionary &p_dict);
	Ref<BattleAction> clone() const;
};

VARIANT_ENUM_CAST(BattleAction::ActionType)

#endif // BATTLE_ACTION_H
