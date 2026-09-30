#ifndef SKILL_DEF_H
#define SKILL_DEF_H

#include "core/io/resource.h"
#include "core/object/ref_counted.h"
#include "core/string/string_name.h"
#include "core/variant/dictionary.h"
#include "core/variant/typed_array.h"

#include "skill_cost.h"
#include "skill_effect.h"

class SkillDef : public Resource {
	GDCLASS(SkillDef, Resource)

	StringName skill_id;
	StringName theme_id;
	int definition_version = 1;
	String display_name;
	String description;
	int target_type = 0;
	int range = 1;
	int min_range = 0;
	int aoe_radius = 0;
	TypedArray<SkillCost> costs;
	TypedArray<SkillEffect> effects;
	StringName cast_animation;
	StringName hit_animation;
	Dictionary terrain_modifiers;
	bool requires_los = false;

protected:
	static void _bind_methods();

public:
	void set_skill_id(const StringName &p_value) { skill_id = p_value; }
	StringName get_skill_id() const { return skill_id; }
	void set_theme_id(const StringName &p_value) { theme_id = p_value; }
	StringName get_theme_id() const { return theme_id; }
	void set_definition_version(int p_value) { definition_version = p_value; }
	int get_definition_version() const { return definition_version; }

	void set_display_name(const String &p_value) { display_name = p_value; }
	String get_display_name() const { return display_name; }

	void set_description(const String &p_value) { description = p_value; }
	String get_description() const { return description; }

	void set_target_type(int p_value) { target_type = p_value; }
	int get_target_type() const { return target_type; }

	void set_range(int p_value) { range = p_value; }
	int get_range() const { return range; }

	void set_min_range(int p_value) { min_range = p_value; }
	int get_min_range() const { return min_range; }

	void set_aoe_radius(int p_value) { aoe_radius = p_value; }
	int get_aoe_radius() const { return aoe_radius; }

	void set_costs(const TypedArray<SkillCost> &p_value) { costs = p_value; }
	TypedArray<SkillCost> get_costs() const { return costs; }

	void set_effects(const TypedArray<SkillEffect> &p_value) { effects = p_value; }
	TypedArray<SkillEffect> get_effects() const { return effects; }

	void set_cast_animation(const StringName &p_value) { cast_animation = p_value; }
	StringName get_cast_animation() const { return cast_animation; }

	void set_hit_animation(const StringName &p_value) { hit_animation = p_value; }
	StringName get_hit_animation() const { return hit_animation; }

	void set_terrain_modifiers(const Dictionary &p_value) { terrain_modifiers = p_value; }
	Dictionary get_terrain_modifiers() const { return terrain_modifiers; }

	void set_requires_los(bool p_value) { requires_los = p_value; }
	bool get_requires_los() const { return requires_los; }
};

#endif // SKILL_DEF_H
