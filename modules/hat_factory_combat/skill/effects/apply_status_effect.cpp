#include "apply_status_effect.h"

#include "../hit_result.h"
#include "../skill_context.h"

#include "core/object/class_db.h"

void ApplyStatusEffect::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_status_id", "status_id"), &ApplyStatusEffect::set_status_id);
	ClassDB::bind_method(D_METHOD("get_status_id"), &ApplyStatusEffect::get_status_id);
	ClassDB::bind_method(D_METHOD("set_stacks", "stacks"), &ApplyStatusEffect::set_stacks);
	ClassDB::bind_method(D_METHOD("get_stacks"), &ApplyStatusEffect::get_stacks);
	ClassDB::bind_method(D_METHOD("set_turns", "turns"), &ApplyStatusEffect::set_turns);
	ClassDB::bind_method(D_METHOD("get_turns"), &ApplyStatusEffect::get_turns);

	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "status_id"), "set_status_id", "get_status_id");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "stacks"), "set_stacks", "get_stacks");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "turns"), "set_turns", "get_turns");
}

void ApplyStatusEffect::apply(const Ref<SkillContext> &p_ctx, TypedArray<HitResult> &r_results) const {
	if (p_ctx.is_null()) {
		return;
	}
	Ref<HitResult> hr;
	hr.instantiate();
	hr->set_target_id(p_ctx->get_target_id());
	hr->set_status_id(status_id);
	hr->set_status_stacks(stacks);
	hr->set_status_turns(turns);
	hr->set_hit(true);
	hr->set_log("Status applied: " + String(status_id));
	r_results.push_back(hr);
}
