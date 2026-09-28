#include "wait_resolver.h"

#include "../action_result.h"
#include "../battle_action.h"
#include "components/turn_component.h"
#include "event/game_event.h"
#include "state/battle_state.h"
#include "state/combat_entity.h"

#include "core/object/class_db.h"

void WaitResolver::_bind_methods() {
	ClassDB::bind_method(D_METHOD("resolve", "state", "action"), &WaitResolver::resolve);
}

Ref<ActionResult> WaitResolver::resolve(const Ref<BattleState> &p_state, const Ref<BattleAction> &p_action) const {
	Ref<ActionResult> result;
	result.instantiate();
	result->set_action(p_action);

	if (p_state.is_null() || p_action.is_null()) {
		result->set_reason("Null state or action");
		return result;
	}

	Ref<BattleState> next = p_state->clone();
	CombatEntity *entity = next->get_entity(p_action->get_actor_id());
	if (entity == nullptr) {
		result->set_reason("Actor does not exist");
		return result;
	}

	Ref<TurnComponent> turn = entity->get_component(StringName("Turn"));
	if (turn.is_valid()) {
		turn->set_moved(true);
		turn->set_acted(true);
	}

	Ref<GameEvent> ev;
	ev.instantiate();
	ev->set_type(GameEvent::TURN_ENDED);
	ev->set_actor_id(p_action->get_actor_id());
	ev->set_round(next->get_round());
	next->append_event(ev);

	result->set_accepted(true);
	result->set_next_state(next);
	result->append_event(ev);
	return result;
}
