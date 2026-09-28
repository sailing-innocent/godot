#include "register_types.h"

#include "core/object/class_db.h"

#include "components/combat_component.h"
#include "components/identity_component.h"
#include "components/stats_component.h"
#include "components/combat_attr_component.h"
#include "components/movement_component.h"
#include "components/vision_component.h"
#include "components/skill_component.h"
#include "components/status_component.h"
#include "components/turn_component.h"
#include "components/transform_component.h"

#include "state/combat_entity.h"
#include "state/battle_state.h"
#include "state/rule_set.h"

#include "action/battle_action.h"
#include "action/action_result.h"
#include "action/action_validator.h"
#include "action/resolvers/move_resolver.h"
#include "action/resolvers/skill_resolver.h"
#include "action/resolvers/wait_resolver.h"
#include "action/resolvers/end_turn_resolver.h"

#include "event/game_event.h"
#include "event/event_log.h"

#include "phase/battle_phase.h"
#include "phase/phase_controller.h"

#include "skill/skill_def.h"
#include "skill/skill_cost.h"
#include "skill/skill_context.h"
#include "skill/skill_executor.h"
#include "skill/hit_result.h"
#include "skill/effects/damage_effect.h"
#include "skill/effects/heal_effect.h"
#include "skill/effects/apply_status_effect.h"

#include "engine/battle_config.h"
#include "engine/battle_engine.h"

#include "query/query_api.h"
#include "serialization/battle_snapshot.h"
#include "serialization/deterministic_random.h"

void initialize_hat_factory_combat_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		GDREGISTER_CLASS(CombatComponent);
		GDREGISTER_CLASS(IdentityComponent);
		GDREGISTER_CLASS(StatsComponent);
		GDREGISTER_CLASS(CombatAttrComponent);
		GDREGISTER_CLASS(MovementComponent);
		GDREGISTER_CLASS(VisionComponent);
		GDREGISTER_CLASS(SkillComponent);
		GDREGISTER_CLASS(StatusComponent);
		GDREGISTER_CLASS(TurnComponent);
		GDREGISTER_CLASS(TransformComponent);

		GDREGISTER_CLASS(CombatEntity);
		GDREGISTER_CLASS(BattleState);
		GDREGISTER_CLASS(RuleSet);

		GDREGISTER_CLASS(BattleAction);
		GDREGISTER_CLASS(ActionResult);
		GDREGISTER_CLASS(ActionValidator);
		GDREGISTER_CLASS(MoveResolver);
		GDREGISTER_CLASS(SkillResolver);
		GDREGISTER_CLASS(WaitResolver);
		GDREGISTER_CLASS(EndTurnResolver);

		GDREGISTER_CLASS(GameEvent);
		GDREGISTER_CLASS(EventLog);

		GDREGISTER_CLASS(BattlePhase);
		GDREGISTER_CLASS(PhaseController);

		GDREGISTER_CLASS(SkillDef);
		GDREGISTER_CLASS(SkillCost);
		GDREGISTER_CLASS(StatusEffectInstance);
		GDREGISTER_CLASS(SkillContext);
		GDREGISTER_CLASS(SkillExecutor);
		GDREGISTER_CLASS(HitResult);
		GDREGISTER_CLASS(DamageEffect);
		GDREGISTER_CLASS(HealEffect);
		GDREGISTER_CLASS(ApplyStatusEffect);

		GDREGISTER_CLASS(BattleConfig);
		GDREGISTER_CLASS(BattleEngine);

		GDREGISTER_CLASS(QueryAPI);
		GDREGISTER_CLASS(BattleSnapshot);
		GDREGISTER_CLASS(DeterministicRandom);
	}
}

void uninitialize_hat_factory_combat_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
}
