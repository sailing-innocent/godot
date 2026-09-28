#include "skill_def.h"

#include "core/object/class_db.h"

void SkillDef::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_skill_id", "skill_id"), &SkillDef::set_skill_id);
	ClassDB::bind_method(D_METHOD("get_skill_id"), &SkillDef::get_skill_id);
	ClassDB::bind_method(D_METHOD("set_display_name", "display_name"), &SkillDef::set_display_name);
	ClassDB::bind_method(D_METHOD("get_display_name"), &SkillDef::get_display_name);
	ClassDB::bind_method(D_METHOD("set_description", "description"), &SkillDef::set_description);
	ClassDB::bind_method(D_METHOD("get_description"), &SkillDef::get_description);
	ClassDB::bind_method(D_METHOD("set_target_type", "target_type"), &SkillDef::set_target_type);
	ClassDB::bind_method(D_METHOD("get_target_type"), &SkillDef::get_target_type);
	ClassDB::bind_method(D_METHOD("set_range", "range"), &SkillDef::set_range);
	ClassDB::bind_method(D_METHOD("get_range"), &SkillDef::get_range);
	ClassDB::bind_method(D_METHOD("set_min_range", "min_range"), &SkillDef::set_min_range);
	ClassDB::bind_method(D_METHOD("get_min_range"), &SkillDef::get_min_range);
	ClassDB::bind_method(D_METHOD("set_aoe_radius", "aoe_radius"), &SkillDef::set_aoe_radius);
	ClassDB::bind_method(D_METHOD("get_aoe_radius"), &SkillDef::get_aoe_radius);
	ClassDB::bind_method(D_METHOD("set_costs", "costs"), &SkillDef::set_costs);
	ClassDB::bind_method(D_METHOD("get_costs"), &SkillDef::get_costs);
	ClassDB::bind_method(D_METHOD("set_effects", "effects"), &SkillDef::set_effects);
	ClassDB::bind_method(D_METHOD("get_effects"), &SkillDef::get_effects);
	ClassDB::bind_method(D_METHOD("set_cast_animation", "cast_animation"), &SkillDef::set_cast_animation);
	ClassDB::bind_method(D_METHOD("get_cast_animation"), &SkillDef::get_cast_animation);
	ClassDB::bind_method(D_METHOD("set_hit_animation", "hit_animation"), &SkillDef::set_hit_animation);
	ClassDB::bind_method(D_METHOD("get_hit_animation"), &SkillDef::get_hit_animation);
	ClassDB::bind_method(D_METHOD("set_terrain_modifiers", "terrain_modifiers"), &SkillDef::set_terrain_modifiers);
	ClassDB::bind_method(D_METHOD("get_terrain_modifiers"), &SkillDef::get_terrain_modifiers);
	ClassDB::bind_method(D_METHOD("set_requires_los", "requires_los"), &SkillDef::set_requires_los);
	ClassDB::bind_method(D_METHOD("get_requires_los"), &SkillDef::get_requires_los);

	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "skill_id"), "set_skill_id", "get_skill_id");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "display_name"), "set_display_name", "get_display_name");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "description"), "set_description", "get_description");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "target_type"), "set_target_type", "get_target_type");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "range"), "set_range", "get_range");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "min_range"), "set_min_range", "get_min_range");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "aoe_radius"), "set_aoe_radius", "get_aoe_radius");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "costs", PROPERTY_HINT_ARRAY_TYPE, "SkillCost"), "set_costs", "get_costs");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "effects", PROPERTY_HINT_ARRAY_TYPE, "SkillEffect"), "set_effects", "get_effects");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "cast_animation"), "set_cast_animation", "get_cast_animation");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "hit_animation"), "set_hit_animation", "get_hit_animation");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "terrain_modifiers"), "set_terrain_modifiers", "get_terrain_modifiers");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "requires_los"), "set_requires_los", "get_requires_los");
}
