#ifndef DAMAGE_EFFECT_H
#define DAMAGE_EFFECT_H

#include "../skill_effect.h"

class DamageEffect : public SkillEffect {
	GDCLASS(DamageEffect, SkillEffect)

	int base_damage = 0;
	bool use_rule_formula = true;

protected:
	static void _bind_methods();

public:
	void set_base_damage(int p_value) { base_damage = p_value; }
	int get_base_damage() const { return base_damage; }

	void set_use_rule_formula(bool p_value) { use_rule_formula = p_value; }
	bool get_use_rule_formula() const { return use_rule_formula; }

	virtual void apply(const Ref<SkillContext> &p_ctx, TypedArray<HitResult> &r_results) const override;
};

#endif // DAMAGE_EFFECT_H
