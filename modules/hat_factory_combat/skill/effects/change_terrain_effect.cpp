#include "change_terrain_effect.h"

#include "../hit_result.h"
#include "../skill_context.h"

#include "core/object/class_db.h"

void ChangeTerrainEffect::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_terrain_id", "terrain_id"), &ChangeTerrainEffect::set_terrain_id);
	ClassDB::bind_method(D_METHOD("get_terrain_id"), &ChangeTerrainEffect::get_terrain_id);
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "terrain_id"), "set_terrain_id", "get_terrain_id");
}

void ChangeTerrainEffect::apply(const Ref<SkillContext> &p_ctx, TypedArray<HitResult> &r_results) const {
	if (p_ctx.is_null() || terrain_id == StringName()) {
		return;
	}
	Ref<HitResult> result;
	result.instantiate();
	result->set_target_id(p_ctx->get_target_id());
	result->set_hit(true);
	result->set_terrain_id(terrain_id);
	result->set_terrain_coord(p_ctx->get_target_coord());
	r_results.push_back(result);
}
