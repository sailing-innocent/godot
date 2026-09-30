#ifndef PUSH_EFFECT_H
#define PUSH_EFFECT_H

#include "../skill_effect.h"

class PushEffect : public SkillEffect {
	GDCLASS(PushEffect, SkillEffect)

	int distance = 1;

protected:
	static void _bind_methods();

public:
	void set_distance(int p_value) { distance = p_value; }
	int get_distance() const { return distance; }
	void apply(const Ref<SkillContext> &p_ctx, TypedArray<HitResult> &r_results) const override;
};

#endif // PUSH_EFFECT_H
