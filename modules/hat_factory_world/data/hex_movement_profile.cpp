/**************************************************************************/
/*  hex_movement_profile.cpp                                              */
/**************************************************************************/
#include "hex_movement_profile.h"

#include "core/object/class_db.h"

void HexMovementProfile::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_profile_id", "id"), &HexMovementProfile::set_profile_id);
	ClassDB::bind_method(D_METHOD("get_profile_id"), &HexMovementProfile::get_profile_id);
	ClassDB::bind_method(D_METHOD("set_size_class", "size"), &HexMovementProfile::set_size_class);
	ClassDB::bind_method(D_METHOD("get_size_class"), &HexMovementProfile::get_size_class);
	ClassDB::bind_method(D_METHOD("set_hat_tags", "tags"), &HexMovementProfile::set_hat_tags);
	ClassDB::bind_method(D_METHOD("get_hat_tags"), &HexMovementProfile::get_hat_tags);
	ClassDB::bind_method(D_METHOD("has_hat_tag", "tag"), &HexMovementProfile::has_hat_tag);
	ClassDB::bind_method(D_METHOD("set_base_move_cost", "cost"), &HexMovementProfile::set_base_move_cost);
	ClassDB::bind_method(D_METHOD("get_base_move_cost"), &HexMovementProfile::get_base_move_cost);
	ClassDB::bind_method(D_METHOD("set_terrain_tag_costs", "costs"), &HexMovementProfile::set_terrain_tag_costs);
	ClassDB::bind_method(D_METHOD("get_terrain_tag_costs"), &HexMovementProfile::get_terrain_tag_costs);
	ClassDB::bind_method(D_METHOD("set_impassable_tags", "tags"), &HexMovementProfile::set_impassable_tags);
	ClassDB::bind_method(D_METHOD("get_impassable_tags"), &HexMovementProfile::get_impassable_tags);
	ClassDB::bind_method(D_METHOD("set_allowed_edge_actions", "actions"), &HexMovementProfile::set_allowed_edge_actions);
	ClassDB::bind_method(D_METHOD("get_allowed_edge_actions"), &HexMovementProfile::get_allowed_edge_actions);
	ClassDB::bind_method(D_METHOD("can_use_edge_action", "action"), &HexMovementProfile::can_use_edge_action);
	ClassDB::bind_method(D_METHOD("set_max_climb_height", "height"), &HexMovementProfile::set_max_climb_height);
	ClassDB::bind_method(D_METHOD("get_max_climb_height"), &HexMovementProfile::get_max_climb_height);
	ClassDB::bind_method(D_METHOD("set_glide_max_drop", "drop"), &HexMovementProfile::set_glide_max_drop);
	ClassDB::bind_method(D_METHOD("get_glide_max_drop"), &HexMovementProfile::get_glide_max_drop);
	ClassDB::bind_method(D_METHOD("set_can_cross_friendly", "enabled"), &HexMovementProfile::set_can_cross_friendly);
	ClassDB::bind_method(D_METHOD("get_can_cross_friendly"), &HexMovementProfile::get_can_cross_friendly);
	ClassDB::bind_method(D_METHOD("get_cost_for_tags", "cell_tags"), &HexMovementProfile::get_cost_for_tags);
	ClassDB::bind_method(D_METHOD("to_dict"), &HexMovementProfile::to_dict);
	ClassDB::bind_static_method("HexMovementProfile", D_METHOD("from_dict", "dict"), &HexMovementProfile::from_dict);
	ClassDB::bind_method(D_METHOD("validate"), &HexMovementProfile::validate);
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "profile_id"), "set_profile_id", "get_profile_id");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "size_class"), "set_size_class", "get_size_class");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_STRING_ARRAY, "hat_tags"), "set_hat_tags", "get_hat_tags");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "base_move_cost"), "set_base_move_cost", "get_base_move_cost");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "terrain_tag_costs"), "set_terrain_tag_costs", "get_terrain_tag_costs");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_STRING_ARRAY, "impassable_tags"), "set_impassable_tags", "get_impassable_tags");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_STRING_ARRAY, "allowed_edge_actions"), "set_allowed_edge_actions", "get_allowed_edge_actions");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_climb_height"), "set_max_climb_height", "get_max_climb_height");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "glide_max_drop"), "set_glide_max_drop", "get_glide_max_drop");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "can_cross_friendly"), "set_can_cross_friendly", "get_can_cross_friendly");
}

void HexMovementProfile::set_profile_id(const StringName &p_id) { profile_id = p_id; }
StringName HexMovementProfile::get_profile_id() const { return profile_id; }
void HexMovementProfile::set_size_class(const StringName &p_size) { size_class = p_size; }
StringName HexMovementProfile::get_size_class() const { return size_class; }
void HexMovementProfile::set_hat_tags(const PackedStringArray &p_tags) { hat_tags = p_tags; }
PackedStringArray HexMovementProfile::get_hat_tags() const { return hat_tags; }

bool HexMovementProfile::has_hat_tag(const StringName &p_tag) const {
	for (int i = 0; i < hat_tags.size(); i++) {
		if (hat_tags[i] == p_tag) {
			return true;
		}
	}
	return false;
}

void HexMovementProfile::set_base_move_cost(int p_cost) { base_move_cost = p_cost; }
int HexMovementProfile::get_base_move_cost() const { return base_move_cost; }
void HexMovementProfile::set_terrain_tag_costs(const Dictionary &p_costs) { terrain_tag_costs = p_costs; }
Dictionary HexMovementProfile::get_terrain_tag_costs() const { return terrain_tag_costs; }
void HexMovementProfile::set_impassable_tags(const PackedStringArray &p_tags) { impassable_tags = p_tags; }
PackedStringArray HexMovementProfile::get_impassable_tags() const { return impassable_tags; }
void HexMovementProfile::set_allowed_edge_actions(const PackedStringArray &p_actions) { allowed_edge_actions = p_actions; }
PackedStringArray HexMovementProfile::get_allowed_edge_actions() const { return allowed_edge_actions; }

bool HexMovementProfile::can_use_edge_action(const StringName &p_action) const {
	for (int i = 0; i < allowed_edge_actions.size(); i++) {
		if (allowed_edge_actions[i] == p_action) {
			return true;
		}
	}
	return false;
}

void HexMovementProfile::set_max_climb_height(int p_height) { max_climb_height = p_height; }
int HexMovementProfile::get_max_climb_height() const { return max_climb_height; }
void HexMovementProfile::set_glide_max_drop(int p_drop) { glide_max_drop = p_drop; }
int HexMovementProfile::get_glide_max_drop() const { return glide_max_drop; }
void HexMovementProfile::set_can_cross_friendly(bool p_enabled) { can_cross_friendly = p_enabled; }
bool HexMovementProfile::get_can_cross_friendly() const { return can_cross_friendly; }

int HexMovementProfile::get_cost_for_tags(const PackedStringArray &p_cell_tags) const {
	for (int i = 0; i < p_cell_tags.size(); i++) {
		for (int j = 0; j < impassable_tags.size(); j++) {
			if (p_cell_tags[i] == impassable_tags[j]) {
				return -1;
			}
		}
	}
	int best = -1;
	for (int i = 0; i < p_cell_tags.size(); i++) {
		if (terrain_tag_costs.has(p_cell_tags[i])) {
			int cost = (int)terrain_tag_costs[p_cell_tags[i]];
			if (best < 0 || cost < best) {
				best = cost;
			}
		}
	}
	return best >= 0 ? best : base_move_cost;
}

Dictionary HexMovementProfile::to_dict() const {
	Dictionary d;
	d[StringName("profile_id")] = profile_id;
	d[StringName("size_class")] = size_class;
	d[StringName("hat_tags")] = hat_tags;
	d[StringName("base_move_cost")] = base_move_cost;
	d[StringName("terrain_tag_costs")] = terrain_tag_costs;
	d[StringName("impassable_tags")] = impassable_tags;
	d[StringName("allowed_edge_actions")] = allowed_edge_actions;
	d[StringName("max_climb_height")] = max_climb_height;
	d[StringName("glide_max_drop")] = glide_max_drop;
	d[StringName("can_cross_friendly")] = can_cross_friendly;
	return d;
}

Ref<HexMovementProfile> HexMovementProfile::from_dict(const Dictionary &p_dict) {
	Ref<HexMovementProfile> p;
	p.instantiate();
	p->set_profile_id(p_dict.get(StringName("profile_id"), StringName()));
	p->set_size_class(p_dict.get(StringName("size_class"), StringName("medium")));
	p->set_hat_tags(p_dict.get(StringName("hat_tags"), PackedStringArray()));
	p->set_base_move_cost(p_dict.get(StringName("base_move_cost"), 1));
	p->set_terrain_tag_costs(p_dict.get(StringName("terrain_tag_costs"), Dictionary()));
	p->set_impassable_tags(p_dict.get(StringName("impassable_tags"), PackedStringArray()));
	p->set_allowed_edge_actions(p_dict.get(StringName("allowed_edge_actions"), PackedStringArray()));
	p->set_max_climb_height(p_dict.get(StringName("max_climb_height"), 1));
	p->set_glide_max_drop(p_dict.get(StringName("glide_max_drop"), 4));
	p->set_can_cross_friendly(p_dict.get(StringName("can_cross_friendly"), true));
	return p;
}

String HexMovementProfile::validate() const {
	if (profile_id == StringName()) {
		return "movement profile missing profile_id";
	}
	if (base_move_cost < 0) {
		return "base_move_cost must be >= 0";
	}
	return String();
}
