#include "game_event.h"

#include "core/object/class_db.h"

void GameEvent::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_type", "type"), &GameEvent::set_type);
	ClassDB::bind_method(D_METHOD("get_type"), &GameEvent::get_type);
	ClassDB::bind_method(D_METHOD("set_actor_id", "actor_id"), &GameEvent::set_actor_id);
	ClassDB::bind_method(D_METHOD("get_actor_id"), &GameEvent::get_actor_id);
	ClassDB::bind_method(D_METHOD("set_target_id", "target_id"), &GameEvent::set_target_id);
	ClassDB::bind_method(D_METHOD("get_target_id"), &GameEvent::get_target_id);
	ClassDB::bind_method(D_METHOD("set_params", "params"), &GameEvent::set_params);
	ClassDB::bind_method(D_METHOD("get_params"), &GameEvent::get_params);
	ClassDB::bind_method(D_METHOD("set_round", "round"), &GameEvent::set_round);
	ClassDB::bind_method(D_METHOD("get_round"), &GameEvent::get_round);

	ClassDB::bind_static_method("GameEvent", D_METHOD("turn_started", "actor"), &GameEvent::turn_started);
	ClassDB::bind_static_method("GameEvent", D_METHOD("unit_moved", "actor", "path"), &GameEvent::unit_moved);
	ClassDB::bind_static_method("GameEvent", D_METHOD("unit_hit", "actor", "target", "damage", "remaining_hp"), &GameEvent::unit_hit);
	ClassDB::bind_static_method("GameEvent", D_METHOD("battle_ended", "winner"), &GameEvent::battle_ended);
	ClassDB::bind_static_method("GameEvent", D_METHOD("item_used", "actor", "target", "item_id"), &GameEvent::item_used);
	ClassDB::bind_static_method("GameEvent", D_METHOD("unit_healed", "actor", "target", "healing", "remaining_hp"), &GameEvent::unit_healed);
	ClassDB::bind_static_method("GameEvent", D_METHOD("unit_died", "target"), &GameEvent::unit_died);
	ClassDB::bind_static_method("GameEvent", D_METHOD("status_applied", "actor", "target", "status_id", "stacks", "turns"), &GameEvent::status_applied);

	ClassDB::bind_method(D_METHOD("to_dict"), &GameEvent::to_dict);
	ClassDB::bind_static_method("GameEvent", D_METHOD("from_dict", "dict"), &GameEvent::from_dict);
	ClassDB::bind_method(D_METHOD("clone"), &GameEvent::clone);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "type"), "set_type", "get_type");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "actor_id"), "set_actor_id", "get_actor_id");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "target_id"), "set_target_id", "get_target_id");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "params"), "set_params", "get_params");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "round"), "set_round", "get_round");

	BIND_ENUM_CONSTANT(TURN_STARTED);
	BIND_ENUM_CONSTANT(TURN_ENDED);
	BIND_ENUM_CONSTANT(ROUND_ENDED);
	BIND_ENUM_CONSTANT(UNIT_MOVED);
	BIND_ENUM_CONSTANT(SKILL_CAST);
	BIND_ENUM_CONSTANT(UNIT_HIT);
	BIND_ENUM_CONSTANT(UNIT_HEALED);
	BIND_ENUM_CONSTANT(STATUS_APPLIED);
	BIND_ENUM_CONSTANT(STATUS_REMOVED);
	BIND_ENUM_CONSTANT(UNIT_DIED);
	BIND_ENUM_CONSTANT(TERRAIN_TRIGGERED);
	BIND_ENUM_CONSTANT(ACTION_REJECTED);
	BIND_ENUM_CONSTANT(BATTLE_ENDED);
	BIND_ENUM_CONSTANT(ITEM_USED);
}

Ref<GameEvent> GameEvent::turn_started(int p_actor) {
	Ref<GameEvent> ev;
	ev.instantiate();
	ev->type = TURN_STARTED;
	ev->actor_id = p_actor;
	return ev;
}

Ref<GameEvent> GameEvent::unit_moved(int p_actor, const TypedArray<Vector2i> &p_path) {
	Ref<GameEvent> ev;
	ev.instantiate();
	ev->type = UNIT_MOVED;
	ev->actor_id = p_actor;
	Dictionary params;
	params["path"] = p_path;
	ev->params = params;
	return ev;
}

Ref<GameEvent> GameEvent::unit_hit(int p_actor, int p_target, int p_damage, int p_remaining_hp) {
	Ref<GameEvent> ev;
	ev.instantiate();
	ev->type = UNIT_HIT;
	ev->actor_id = p_actor;
	ev->target_id = p_target;
	Dictionary params;
	params["damage"] = p_damage;
	params["remaining_hp"] = p_remaining_hp;
	ev->params = params;
	return ev;
}

Ref<GameEvent> GameEvent::battle_ended(const StringName &p_winner) {
	Ref<GameEvent> ev;
	ev.instantiate();
	ev->type = BATTLE_ENDED;
	Dictionary params;
	params["winner"] = p_winner;
	ev->params = params;
	return ev;
}

Ref<GameEvent> GameEvent::item_used(int p_actor, int p_target, const StringName &p_item_id) {
	Ref<GameEvent> ev;
	ev.instantiate();
	ev->type = ITEM_USED;
	ev->actor_id = p_actor;
	ev->target_id = p_target;
	Dictionary params;
	params["item_id"] = p_item_id;
	ev->params = params;
	return ev;
}

Ref<GameEvent> GameEvent::unit_healed(int p_actor, int p_target, int p_healing, int p_remaining_hp) {
	Ref<GameEvent> ev;
	ev.instantiate();
	ev->type = UNIT_HEALED;
	ev->actor_id = p_actor;
	ev->target_id = p_target;
	Dictionary params;
	params["healing"] = p_healing;
	params["remaining_hp"] = p_remaining_hp;
	ev->params = params;
	return ev;
}

Ref<GameEvent> GameEvent::unit_died(int p_target) {
	Ref<GameEvent> ev;
	ev.instantiate();
	ev->type = UNIT_DIED;
	ev->target_id = p_target;
	return ev;
}

Ref<GameEvent> GameEvent::status_applied(int p_actor, int p_target, const StringName &p_status_id, int p_stacks, int p_turns) {
	Ref<GameEvent> ev;
	ev.instantiate();
	ev->type = STATUS_APPLIED;
	ev->actor_id = p_actor;
	ev->target_id = p_target;
	Dictionary params;
	params["status_id"] = p_status_id;
	params["stacks"] = p_stacks;
	params["turns"] = p_turns;
	ev->params = params;
	return ev;
}

Dictionary GameEvent::to_dict() const {
	Dictionary d;
	d["type"] = type;
	d["actor_id"] = actor_id;
	d["target_id"] = target_id;
	d["params"] = params;
	d["round"] = round;
	return d;
}

Ref<GameEvent> GameEvent::from_dict(const Dictionary &p_dict) {
	Ref<GameEvent> ev;
	ev.instantiate();
	if (p_dict.has("type")) ev->type = p_dict["type"];
	if (p_dict.has("actor_id")) ev->actor_id = p_dict["actor_id"];
	if (p_dict.has("target_id")) ev->target_id = p_dict["target_id"];
	if (p_dict.has("params")) ev->params = p_dict["params"];
	if (p_dict.has("round")) ev->round = p_dict["round"];
	return ev;
}

Ref<GameEvent> GameEvent::clone() const {
	Ref<GameEvent> ev;
	ev.instantiate();
	ev->type = type;
	ev->actor_id = actor_id;
	ev->target_id = target_id;
	ev->params = params.duplicate();
	ev->round = round;
	return ev;
}
