#include "hex_transition_def.h"

#include "core/object/class_db.h"

void HexTransitionDef::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_source_tags", "source_tags"), &HexTransitionDef::set_source_tags);
	ClassDB::bind_method(D_METHOD("get_source_tags"), &HexTransitionDef::get_source_tags);

	ClassDB::bind_method(D_METHOD("set_target_tags", "target_tags"), &HexTransitionDef::set_target_tags);
	ClassDB::bind_method(D_METHOD("get_target_tags"), &HexTransitionDef::get_target_tags);

	ClassDB::bind_method(D_METHOD("set_source_terrain_id", "source_terrain_id"), &HexTransitionDef::set_source_terrain_id);
	ClassDB::bind_method(D_METHOD("get_source_terrain_id"), &HexTransitionDef::get_source_terrain_id);

	ClassDB::bind_method(D_METHOD("set_target_terrain_id", "target_terrain_id"), &HexTransitionDef::set_target_terrain_id);
	ClassDB::bind_method(D_METHOD("get_target_terrain_id"), &HexTransitionDef::get_target_terrain_id);

	ClassDB::bind_method(D_METHOD("set_min_height_delta", "min_height_delta"), &HexTransitionDef::set_min_height_delta);
	ClassDB::bind_method(D_METHOD("get_min_height_delta"), &HexTransitionDef::get_min_height_delta);

	ClassDB::bind_method(D_METHOD("set_max_height_delta", "max_height_delta"), &HexTransitionDef::set_max_height_delta);
	ClassDB::bind_method(D_METHOD("get_max_height_delta"), &HexTransitionDef::get_max_height_delta);

	ClassDB::bind_method(D_METHOD("set_require_same_terrain", "require_same_terrain"), &HexTransitionDef::set_require_same_terrain);
	ClassDB::bind_method(D_METHOD("get_require_same_terrain"), &HexTransitionDef::get_require_same_terrain);

	ClassDB::bind_method(D_METHOD("set_require_different_terrain", "require_different_terrain"), &HexTransitionDef::set_require_different_terrain);
	ClassDB::bind_method(D_METHOD("get_require_different_terrain"), &HexTransitionDef::get_require_different_terrain);

	ClassDB::bind_method(D_METHOD("set_probability", "probability"), &HexTransitionDef::set_probability);
	ClassDB::bind_method(D_METHOD("get_probability"), &HexTransitionDef::get_probability);

	ClassDB::bind_method(D_METHOD("set_priority", "priority"), &HexTransitionDef::set_priority);
	ClassDB::bind_method(D_METHOD("get_priority"), &HexTransitionDef::get_priority);

	ClassDB::bind_method(D_METHOD("set_profile", "profile"), &HexTransitionDef::set_profile);
	ClassDB::bind_method(D_METHOD("get_profile"), &HexTransitionDef::get_profile);

	ClassDB::bind_method(D_METHOD("set_gameplay_flags", "gameplay_flags"), &HexTransitionDef::set_gameplay_flags);
	ClassDB::bind_method(D_METHOD("get_gameplay_flags"), &HexTransitionDef::get_gameplay_flags);

	ClassDB::bind_method(D_METHOD("set_gameplay_state_id", "gameplay_state_id"), &HexTransitionDef::set_gameplay_state_id);
	ClassDB::bind_method(D_METHOD("get_gameplay_state_id"), &HexTransitionDef::get_gameplay_state_id);

	ADD_PROPERTY(PropertyInfo(Variant::PACKED_STRING_ARRAY, "source_tags"), "set_source_tags", "get_source_tags");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_STRING_ARRAY, "target_tags"), "set_target_tags", "get_target_tags");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "source_terrain_id"), "set_source_terrain_id", "get_source_terrain_id");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "target_terrain_id"), "set_target_terrain_id", "get_target_terrain_id");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "min_height_delta"), "set_min_height_delta", "get_min_height_delta");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_height_delta"), "set_max_height_delta", "get_max_height_delta");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "require_same_terrain"), "set_require_same_terrain", "get_require_same_terrain");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "require_different_terrain"), "set_require_different_terrain", "get_require_different_terrain");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "probability"), "set_probability", "get_probability");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "priority"), "set_priority", "get_priority");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "profile", PROPERTY_HINT_RESOURCE_TYPE, "HexTransitionProfile"), "set_profile", "get_profile");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "gameplay_flags"), "set_gameplay_flags", "get_gameplay_flags");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "gameplay_state_id"), "set_gameplay_state_id", "get_gameplay_state_id");
}

void HexTransitionDef::set_source_tags(const PackedStringArray &p_tags) {
	source_tags = p_tags;
}

PackedStringArray HexTransitionDef::get_source_tags() const {
	return source_tags;
}

void HexTransitionDef::set_target_tags(const PackedStringArray &p_tags) {
	target_tags = p_tags;
}

PackedStringArray HexTransitionDef::get_target_tags() const {
	return target_tags;
}

void HexTransitionDef::set_source_terrain_id(const StringName &p_id) {
	source_terrain_id = p_id;
}

StringName HexTransitionDef::get_source_terrain_id() const {
	return source_terrain_id;
}

void HexTransitionDef::set_target_terrain_id(const StringName &p_id) {
	target_terrain_id = p_id;
}

StringName HexTransitionDef::get_target_terrain_id() const {
	return target_terrain_id;
}

void HexTransitionDef::set_min_height_delta(int p_delta) {
	min_height_delta = p_delta;
}

int HexTransitionDef::get_min_height_delta() const {
	return min_height_delta;
}

void HexTransitionDef::set_max_height_delta(int p_delta) {
	max_height_delta = p_delta;
}

int HexTransitionDef::get_max_height_delta() const {
	return max_height_delta;
}

void HexTransitionDef::set_require_same_terrain(bool p_enabled) {
	require_same_terrain = p_enabled;
}

bool HexTransitionDef::get_require_same_terrain() const {
	return require_same_terrain;
}

void HexTransitionDef::set_require_different_terrain(bool p_enabled) {
	require_different_terrain = p_enabled;
}

bool HexTransitionDef::get_require_different_terrain() const {
	return require_different_terrain;
}

void HexTransitionDef::set_probability(float p_probability) {
	probability = p_probability;
}

float HexTransitionDef::get_probability() const {
	return probability;
}

void HexTransitionDef::set_priority(int p_priority) {
	priority = p_priority;
}

int HexTransitionDef::get_priority() const {
	return priority;
}

void HexTransitionDef::set_profile(const Ref<HexTransitionProfile> &p_profile) {
	profile = p_profile;
}

Ref<HexTransitionProfile> HexTransitionDef::get_profile() const {
	return profile;
}

void HexTransitionDef::set_gameplay_flags(uint32_t p_flags) {
	gameplay_flags = p_flags;
}

uint32_t HexTransitionDef::get_gameplay_flags() const {
	return gameplay_flags;
}

void HexTransitionDef::set_gameplay_state_id(const StringName &p_id) {
	gameplay_state_id = p_id;
}

StringName HexTransitionDef::get_gameplay_state_id() const {
	return gameplay_state_id;
}
