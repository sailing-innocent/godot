#ifndef CHANGE_TERRAIN_EFFECT_H
#define CHANGE_TERRAIN_EFFECT_H

#include "../skill_effect.h"

class ChangeTerrainEffect : public SkillEffect {
	GDCLASS(ChangeTerrainEffect, SkillEffect)

	StringName terrain_id;

protected:
	static void _bind_methods();

public:
	void set_terrain_id(const StringName &p_value) { terrain_id = p_value; }
	StringName get_terrain_id() const { return terrain_id; }
	bool can_apply_without_target() const override { return true; }
	void apply(const Ref<SkillContext> &p_ctx, TypedArray<HitResult> &r_results) const override;
};

#endif // CHANGE_TERRAIN_EFFECT_H
