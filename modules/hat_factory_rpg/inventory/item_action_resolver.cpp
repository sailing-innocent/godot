#include "item_action_resolver.h"

#include "item/item_def.h"
#include "item/item_effect.h"

#include "modules/hat_factory_combat/action/action_result.h"
#include "modules/hat_factory_combat/action/battle_action.h"
#include "modules/hat_factory_combat/components/stats_component.h"
#include "modules/hat_factory_combat/components/turn_component.h"
#include "modules/hat_factory_combat/event/game_event.h"
#include "modules/hat_factory_combat/state/battle_state.h"
#include "modules/hat_factory_combat/state/combat_entity.h"

#include "core/io/resource_loader.h"
#include "core/object/class_db.h"

void ItemActionResolver::_bind_methods() {
	ClassDB::bind_method(D_METHOD("resolve", "state", "action"), &ItemActionResolver::resolve);
}

Ref<ActionResult> ItemActionResolver::resolve(const Ref<BattleState> &p_state, const Ref<BattleAction> &p_action) const {
	Ref<ActionResult> result;
	result.instantiate();
	result->set_action(p_action);

	if (p_state.is_null() || p_action.is_null()) {
		result->set_reason("Null state or action");
		return result;
	}

	Dictionary payload = p_action->get_payload();
	StringName item_id = payload.get("item_id", StringName());
	if (item_id == StringName()) {
		result->set_reason("Missing item_id");
		return result;
	}

	String path = "res://resources/items/" + String(item_id) + ".tres";
	Ref<ItemDef> def = ResourceLoader::load(path);
	if (def.is_null()) {
		result->set_reason("Unknown item: " + String(item_id));
		return result;
	}

	int target_id = payload.get("target_entity", 0);
	if (target_id == 0) {
		target_id = p_action->get_actor_id();
	}

	Ref<BattleState> next = p_state->clone();
	CombatEntity *target = next->get_entity(target_id);
	if (target == nullptr) {
		result->set_reason("Target does not exist");
		return result;
	}

	CombatEntity *actor = next->get_entity(p_action->get_actor_id());
	if (actor != nullptr) {
		Ref<TurnComponent> turn = actor->get_component(StringName("Turn"));
		if (turn.is_valid()) {
			turn->set_acted(true);
		}
	}

	// Apply effects and collect feedback events.
	TypedArray<ItemEffect> effects = def->get_effects();
	int total_healing = 0;
	for (int i = 0; i < effects.size(); i++) {
		Ref<ItemEffect> eff = effects[i];
		if (eff.is_null()) {
			continue;
		}
		eff->apply(Ref<CombatEntity>(target));
		if (eff->get_effect_type() == ItemEffect::EFFECT_HEAL_HP) {
			total_healing += eff->get_value();
		}
	}

	Ref<GameEvent> use_ev = GameEvent::item_used(p_action->get_actor_id(), target_id, item_id);
	use_ev->set_round(next->get_round());
	next->append_event(use_ev);
	result->append_event(use_ev);

	if (total_healing > 0) {
		int remaining_hp = 0;
		Ref<StatsComponent> stats = target->get_component(StringName("Stats"));
		if (stats.is_valid()) {
			remaining_hp = stats->get_hp();
		}
		Ref<GameEvent> heal_ev = GameEvent::unit_healed(p_action->get_actor_id(), target_id, total_healing, remaining_hp);
		heal_ev->set_round(next->get_round());
		next->append_event(heal_ev);
		result->append_event(heal_ev);
	}

	result->set_accepted(true);
	result->set_next_state(next);
	return result;
}
