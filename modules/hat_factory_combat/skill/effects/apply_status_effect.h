#ifndef APPLY_STATUS_EFFECT_H
#define APPLY_STATUS_EFFECT_H

#include "../skill_effect.h"

class ApplyStatusEffect : public SkillEffect {
	GDCLASS(ApplyStatusEffect, SkillEffect)

	StringName status_id;
	int stacks = 1;
	int turns = 3;

protected:
	static void _bind_methods();

public:
	void set_status_id(const StringName &p_value) { status_id = p_value; }
	StringName get_status_id() const { return status_id; }

	void set_stacks(int p_value) { stacks = p_value; }
	int get_stacks() const { return stacks; }

	void set_turns(int p_value) { turns = p_value; }
	int get_turns() const { return turns; }

	virtual void apply(const Ref<SkillContext> &p_ctx, TypedArray<HitResult> &r_results) const override;
};

#endif // APPLY_STATUS_EFFECT_H
