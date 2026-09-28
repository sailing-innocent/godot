#ifndef SKILL_RESOLVER_H
#define SKILL_RESOLVER_H

#include "core/math/vector2i.h"
#include "core/object/ref_counted.h"
#include "core/variant/typed_array.h"

class ActionResult;
class BattleAction;
class BattleState;
class CombatEntity;
class HitResult;
class SkillDef;

class SkillResolver : public RefCounted {
	GDCLASS(SkillResolver, RefCounted)

	TypedArray<int> _gather_targets(const Ref<BattleState> &p_state, const Ref<CombatEntity> &p_caster, const Ref<SkillDef> &p_def, const Vector2i &p_target_coord, int p_target_entity) const;
	void _apply_hit_results(const Ref<BattleState> &p_state, const Ref<ActionResult> &p_result, int p_caster_id, int p_target_id, const TypedArray<HitResult> &p_hits, TypedArray<int> &r_died) const;

protected:
	static void _bind_methods();

public:
	Ref<ActionResult> resolve(const Ref<BattleState> &p_state, const Ref<BattleAction> &p_action) const;
};

#endif // SKILL_RESOLVER_H
