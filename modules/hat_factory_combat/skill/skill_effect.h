#ifndef SKILL_EFFECT_H
#define SKILL_EFFECT_H

#include "core/io/resource.h"
#include "core/variant/typed_array.h"

class HitResult;
class SkillContext;

class SkillEffect : public Resource {
	GDCLASS(SkillEffect, Resource)

protected:
	static void _bind_methods();

public:
	virtual void apply(const Ref<SkillContext> &p_ctx, TypedArray<HitResult> &r_results) const = 0;
	virtual bool can_apply_without_target() const { return false; }
};

#endif // SKILL_EFFECT_H
