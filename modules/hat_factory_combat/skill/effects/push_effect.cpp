#include "push_effect.h"

#include "../hit_result.h"
#include "../skill_context.h"
#include "components/transform_component.h"
#include "state/combat_entity.h"
#include "query/query_api.h"

#include "core/object/class_db.h"

void PushEffect::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_distance", "distance"), &PushEffect::set_distance);
	ClassDB::bind_method(D_METHOD("get_distance"), &PushEffect::get_distance);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "distance"), "set_distance", "get_distance");
}

void PushEffect::apply(const Ref<SkillContext> &p_ctx, TypedArray<HitResult> &r_results) const {
	if (p_ctx.is_null() || p_ctx->get_state().is_null() || distance < 1) {
		return;
	}
	CombatEntity *caster = p_ctx->get_state()->get_entity(p_ctx->get_caster_id());
	CombatEntity *target = p_ctx->get_state()->get_entity(p_ctx->get_target_id());
	if (caster == nullptr || target == nullptr) {
		return;
	}
	Ref<TransformComponent> source = caster->get_component(StringName("Transform"));
	Ref<TransformComponent> destination = target->get_component(StringName("Transform"));
	if (source.is_null() || destination.is_null()) {
		return;
	}
	Vector2i direction = destination->get_coord() - source->get_coord();
	if (QueryAPI::hex_distance(Vector2i(), direction) != 1) {
		return;
	}
	Ref<HitResult> result;
	result.instantiate();
	result->set_target_id(p_ctx->get_target_id());
	result->set_hit(true);
	result->set_displacement(destination->get_coord() + direction * distance);
	r_results.push_back(result);
}
