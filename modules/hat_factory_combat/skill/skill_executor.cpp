#include "skill_executor.h"

#include "hit_result.h"
#include "skill_context.h"
#include "skill_def.h"
#include "skill_effect.h"

#include "core/object/class_db.h"

void SkillExecutor::_bind_methods() {
	ClassDB::bind_method(D_METHOD("execute", "context"), &SkillExecutor::execute);
}

TypedArray<HitResult> SkillExecutor::execute(const Ref<SkillContext> &p_ctx) const {
	TypedArray<HitResult> results;
	if (p_ctx.is_null() || p_ctx->get_skill().is_null()) {
		return results;
	}
	TypedArray<SkillEffect> effects = p_ctx->get_skill()->get_effects();
	for (int i = 0; i < effects.size(); i++) {
		Ref<SkillEffect> effect = effects[i];
		if (effect.is_valid()) {
			effect->apply(p_ctx, results);
		}
	}
	return results;
}
