#ifndef SKILL_CONTEXT_H
#define SKILL_CONTEXT_H

#include "core/math/vector2i.h"
#include "core/object/ref_counted.h"

#include "modules/hat_factory_combat/state/battle_state.h"
#include "skill_def.h"
#include "modules/hat_factory_combat/state/rule_set.h"

class SkillContext : public RefCounted {
	GDCLASS(SkillContext, RefCounted)

	Ref<BattleState> state;
	int caster_id = 0;
	int target_id = 0;
	Vector2i target_coord;
	Ref<SkillDef> skill;
	Ref<RuleSet> rules;

protected:
	static void _bind_methods();

public:
	void set_state(const Ref<BattleState> &p_value) { state = p_value; }
	Ref<BattleState> get_state() const { return state; }

	void set_caster_id(int p_value) { caster_id = p_value; }
	int get_caster_id() const { return caster_id; }

	void set_target_id(int p_value) { target_id = p_value; }
	int get_target_id() const { return target_id; }

	void set_target_coord(const Vector2i &p_value) { target_coord = p_value; }
	Vector2i get_target_coord() const { return target_coord; }

	void set_skill(const Ref<SkillDef> &p_value) { skill = p_value; }
	Ref<SkillDef> get_skill() const { return skill; }

	void set_rules(const Ref<RuleSet> &p_value) { rules = p_value; }
	Ref<RuleSet> get_rules() const { return rules; }
};

#endif // SKILL_CONTEXT_H
