#ifndef HEAL_EFFECT_H
#define HEAL_EFFECT_H

#include "../skill_effect.h"

class HealEffect : public SkillEffect {
	GDCLASS(HealEffect, SkillEffect)

	int base_healing = 0;

protected:
	static void _bind_methods();

public:
	void set_base_healing(int p_value) { base_healing = p_value; }
	int get_base_healing() const { return base_healing; }

	virtual void apply(const Ref<SkillContext> &p_ctx, TypedArray<HitResult> &r_results) const override;
};

#endif // HEAL_EFFECT_H
