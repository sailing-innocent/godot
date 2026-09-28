#include "heal_effect.h"

#include "../hit_result.h"
#include "../skill_context.h"

#include "core/object/class_db.h"

void HealEffect::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_base_healing", "base_healing"), &HealEffect::set_base_healing);
	ClassDB::bind_method(D_METHOD("get_base_healing"), &HealEffect::get_base_healing);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "base_healing"), "set_base_healing", "get_base_healing");
}

void HealEffect::apply(const Ref<SkillContext> &p_ctx, TypedArray<HitResult> &r_results) const {
	if (p_ctx.is_null()) {
		return;
	}
	Ref<HitResult> hr;
	hr.instantiate();
	hr->set_target_id(p_ctx->get_target_id());
	hr->set_healing(base_healing);
	hr->set_hit(true);
	hr->set_log("Heal: " + String::num_int64(base_healing));
	r_results.push_back(hr);
}
