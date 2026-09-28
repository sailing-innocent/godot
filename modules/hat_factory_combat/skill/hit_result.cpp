#include "hit_result.h"

#include "core/object/class_db.h"

void HitResult::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_target_id", "target_id"), &HitResult::set_target_id);
	ClassDB::bind_method(D_METHOD("get_target_id"), &HitResult::get_target_id);
	ClassDB::bind_method(D_METHOD("set_damage", "damage"), &HitResult::set_damage);
	ClassDB::bind_method(D_METHOD("get_damage"), &HitResult::get_damage);
	ClassDB::bind_method(D_METHOD("set_healing", "healing"), &HitResult::set_healing);
	ClassDB::bind_method(D_METHOD("get_healing"), &HitResult::get_healing);
	ClassDB::bind_method(D_METHOD("set_hit", "hit"), &HitResult::set_hit);
	ClassDB::bind_method(D_METHOD("get_hit"), &HitResult::get_hit);
	ClassDB::bind_method(D_METHOD("set_status_id", "status_id"), &HitResult::set_status_id);
	ClassDB::bind_method(D_METHOD("get_status_id"), &HitResult::get_status_id);
	ClassDB::bind_method(D_METHOD("set_status_stacks", "status_stacks"), &HitResult::set_status_stacks);
	ClassDB::bind_method(D_METHOD("get_status_stacks"), &HitResult::get_status_stacks);
	ClassDB::bind_method(D_METHOD("set_status_turns", "status_turns"), &HitResult::set_status_turns);
	ClassDB::bind_method(D_METHOD("get_status_turns"), &HitResult::get_status_turns);
	ClassDB::bind_method(D_METHOD("set_log", "log"), &HitResult::set_log);
	ClassDB::bind_method(D_METHOD("get_log"), &HitResult::get_log);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "target_id"), "set_target_id", "get_target_id");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "damage"), "set_damage", "get_damage");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "healing"), "set_healing", "get_healing");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "hit"), "set_hit", "get_hit");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "status_id"), "set_status_id", "get_status_id");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "status_stacks"), "set_status_stacks", "get_status_stacks");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "status_turns"), "set_status_turns", "get_status_turns");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "log"), "set_log", "get_log");
}
