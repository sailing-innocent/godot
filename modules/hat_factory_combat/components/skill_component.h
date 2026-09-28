#ifndef SKILL_COMPONENT_H
#define SKILL_COMPONENT_H

#include "combat_component.h"
#include "core/variant/typed_array.h"

class SkillComponent : public CombatComponent {
	GDCLASS(SkillComponent, CombatComponent)

	TypedArray<StringName> skill_ids;
	Dictionary cooldowns;

protected:
	static void _bind_methods();

public:
	virtual StringName get_component_name() const override { return StringName("Skill"); }

	void set_skill_ids(const TypedArray<StringName> &p_value) { skill_ids = p_value; }
	TypedArray<StringName> get_skill_ids() const { return skill_ids; }

	void set_cooldowns(const Dictionary &p_value) { cooldowns = p_value; }
	Dictionary get_cooldowns() const { return cooldowns; }

	void set_cooldown(const StringName &p_skill_id, int p_turns);
	int get_cooldown(const StringName &p_skill_id) const;
	void tick_cooldowns();

	virtual Dictionary to_dict() const override;
	virtual void from_dict(const Dictionary &p_dict) override;
	virtual Ref<CombatComponent> clone() const override;
};

#endif // SKILL_COMPONENT_H
