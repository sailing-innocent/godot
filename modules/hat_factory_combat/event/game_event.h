#ifndef GAME_EVENT_H
#define GAME_EVENT_H

#include "core/math/vector2i.h"
#include "core/object/ref_counted.h"
#include "core/string/string_name.h"
#include "core/variant/dictionary.h"
#include "core/variant/typed_array.h"

class GameEvent : public RefCounted {
	GDCLASS(GameEvent, RefCounted)

public:
	enum GameEventType {
		TURN_STARTED = 0,
		TURN_ENDED = 1,
		ROUND_ENDED = 2,
		UNIT_MOVED = 3,
		SKILL_CAST = 4,
		UNIT_HIT = 5,
		UNIT_HEALED = 6,
		STATUS_APPLIED = 7,
		STATUS_REMOVED = 8,
		UNIT_DIED = 9,
		TERRAIN_TRIGGERED = 10,
		ACTION_REJECTED = 11,
		BATTLE_ENDED = 12,
		ITEM_USED = 13,
		TERRAIN_CHANGED = 14,
	};

private:
	int type = TURN_STARTED;
	int actor_id = 0;
	int target_id = 0;
	Dictionary params;
	int round = 0;

protected:
	static void _bind_methods();

public:
	void set_type(int p_value) { type = p_value; }
	int get_type() const { return type; }

	void set_actor_id(int p_value) { actor_id = p_value; }
	int get_actor_id() const { return actor_id; }

	void set_target_id(int p_value) { target_id = p_value; }
	int get_target_id() const { return target_id; }

	void set_params(const Dictionary &p_value) { params = p_value; }
	Dictionary get_params() const { return params; }

	void set_round(int p_value) { round = p_value; }
	int get_round() const { return round; }

	static Ref<GameEvent> turn_started(int p_actor);
	static Ref<GameEvent> unit_moved(int p_actor, const TypedArray<Vector2i> &p_path);
	static Ref<GameEvent> unit_hit(int p_actor, int p_target, int p_damage, int p_remaining_hp);
	static Ref<GameEvent> battle_ended(const StringName &p_winner);
	static Ref<GameEvent> item_used(int p_actor, int p_target, const StringName &p_item_id);
	static Ref<GameEvent> unit_healed(int p_actor, int p_target, int p_healing, int p_remaining_hp);
	static Ref<GameEvent> unit_died(int p_target);
	static Ref<GameEvent> status_applied(int p_actor, int p_target, const StringName &p_status_id, int p_stacks, int p_turns);

	Dictionary to_dict() const;
	static Ref<GameEvent> from_dict(const Dictionary &p_dict);
	Ref<GameEvent> clone() const;
};

VARIANT_ENUM_CAST(GameEvent::GameEventType)

#endif // GAME_EVENT_H
