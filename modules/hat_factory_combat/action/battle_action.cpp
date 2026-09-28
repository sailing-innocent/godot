#include "battle_action.h"

#include "core/object/class_db.h"
#include "core/variant/typed_array.h"

void BattleAction::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_action_id", "action_id"), &BattleAction::set_action_id);
	ClassDB::bind_method(D_METHOD("get_action_id"), &BattleAction::get_action_id);
	ClassDB::bind_method(D_METHOD("set_type", "type"), &BattleAction::set_type);
	ClassDB::bind_method(D_METHOD("get_type"), &BattleAction::get_type);
	ClassDB::bind_method(D_METHOD("set_actor_id", "actor_id"), &BattleAction::set_actor_id);
	ClassDB::bind_method(D_METHOD("get_actor_id"), &BattleAction::get_actor_id);
	ClassDB::bind_method(D_METHOD("set_payload", "payload"), &BattleAction::set_payload);
	ClassDB::bind_method(D_METHOD("get_payload"), &BattleAction::get_payload);

	ClassDB::bind_static_method("BattleAction", D_METHOD("move", "actor", "path"), &BattleAction::move);
	ClassDB::bind_static_method("BattleAction", D_METHOD("skill", "actor", "skill_id", "target", "target_entity"), &BattleAction::skill, DEFVAL(0));
	ClassDB::bind_static_method("BattleAction", D_METHOD("item", "actor", "item_id", "target_entity"), &BattleAction::item, DEFVAL(0));
	ClassDB::bind_static_method("BattleAction", D_METHOD("wait", "actor"), &BattleAction::wait);
	ClassDB::bind_static_method("BattleAction", D_METHOD("end_turn", "actor"), &BattleAction::end_turn);
	ClassDB::bind_static_method("BattleAction", D_METHOD("escape", "actor"), &BattleAction::escape);

	ClassDB::bind_method(D_METHOD("to_dict"), &BattleAction::to_dict);
	ClassDB::bind_static_method("BattleAction", D_METHOD("from_dict", "dict"), &BattleAction::from_dict);
	ClassDB::bind_method(D_METHOD("clone"), &BattleAction::clone);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "action_id"), "set_action_id", "get_action_id");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "type"), "set_type", "get_type");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "actor_id"), "set_actor_id", "get_actor_id");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "payload"), "set_payload", "get_payload");

	BIND_ENUM_CONSTANT(MOVE);
	BIND_ENUM_CONSTANT(SKILL);
	BIND_ENUM_CONSTANT(ITEM);
	BIND_ENUM_CONSTANT(WAIT);
	BIND_ENUM_CONSTANT(END_TURN);
	BIND_ENUM_CONSTANT(ESCAPE);
}

Ref<BattleAction> BattleAction::move(int p_actor, const TypedArray<Vector2i> &p_path) {
	Ref<BattleAction> a;
	a.instantiate();
	a->type = MOVE;
	a->actor_id = p_actor;
	Dictionary pl;
	pl["path"] = p_path;
	if (p_path.size() > 0) {
		pl["target"] = p_path[p_path.size() - 1];
	}
	a->payload = pl;
	return a;
}

Ref<BattleAction> BattleAction::skill(int p_actor, const StringName &p_skill_id, const Vector2i &p_target, int p_target_entity) {
	Ref<BattleAction> a;
	a.instantiate();
	a->type = SKILL;
	a->actor_id = p_actor;
	Dictionary pl;
	pl["skill_id"] = p_skill_id;
	pl["target"] = p_target;
	pl["target_entity"] = p_target_entity;
	a->payload = pl;
	return a;
}

Ref<BattleAction> BattleAction::item(int p_actor, const StringName &p_item_id, int p_target_entity) {
	Ref<BattleAction> a;
	a.instantiate();
	a->type = ITEM;
	a->actor_id = p_actor;
	Dictionary pl;
	pl["item_id"] = p_item_id;
	pl["target_entity"] = p_target_entity;
	a->payload = pl;
	return a;
}

Ref<BattleAction> BattleAction::wait(int p_actor) {
	Ref<BattleAction> a;
	a.instantiate();
	a->type = WAIT;
	a->actor_id = p_actor;
	return a;
}

Ref<BattleAction> BattleAction::end_turn(int p_actor) {
	Ref<BattleAction> a;
	a.instantiate();
	a->type = END_TURN;
	a->actor_id = p_actor;
	return a;
}

Ref<BattleAction> BattleAction::escape(int p_actor) {
	Ref<BattleAction> a;
	a.instantiate();
	a->type = ESCAPE;
	a->actor_id = p_actor;
	return a;
}

Dictionary BattleAction::to_dict() const {
	Dictionary d;
	d["action_id"] = action_id;
	d["type"] = type;
	d["actor_id"] = actor_id;
	d["payload"] = payload;
	return d;
}

Ref<BattleAction> BattleAction::from_dict(const Dictionary &p_dict) {
	Ref<BattleAction> a;
	a.instantiate();
	if (p_dict.has("action_id")) a->action_id = p_dict["action_id"];
	if (p_dict.has("type")) a->type = p_dict["type"];
	if (p_dict.has("actor_id")) a->actor_id = p_dict["actor_id"];
	if (p_dict.has("payload")) a->payload = p_dict["payload"];
	return a;
}

Ref<BattleAction> BattleAction::clone() const {
	Ref<BattleAction> a;
	a.instantiate();
	a->action_id = action_id;
	a->type = type;
	a->actor_id = actor_id;
	a->payload = payload.duplicate();
	return a;
}
