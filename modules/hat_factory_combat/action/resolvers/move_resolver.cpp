#include "move_resolver.h"

#include "../action_result.h"
#include "../battle_action.h"
#include "components/transform_component.h"
#include "components/turn_component.h"
#include "event/game_event.h"
#include "state/battle_state.h"
#include "state/combat_entity.h"

#include "core/object/class_db.h"

void MoveResolver::_bind_methods() {
	ClassDB::bind_method(D_METHOD("resolve", "state", "action"), &MoveResolver::resolve);
}

Ref<ActionResult> MoveResolver::resolve(const Ref<BattleState> &p_state, const Ref<BattleAction> &p_action) const {
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

	Dictionary payload = p_action->get_payload();
	Vector2i target = payload.get("target", Vector2i());

	Ref<TransformComponent> transform = entity->get_component(StringName("Transform"));
	if (transform.is_valid()) {
		transform->set_coord(target);
	}

	Ref<TurnComponent> turn = entity->get_component(StringName("Turn"));
	if (turn.is_valid()) {
		turn->set_moved(true);
	}

	TypedArray<Vector2i> path;
	if (payload.has("path")) {
		path = TypedArray<Vector2i>(payload["path"]);
	} else {
		path.push_back(target);
	}

	Ref<GameEvent> ev = GameEvent::unit_moved(p_action->get_actor_id(), path);
	ev->set_round(next->get_round());
	next->append_event(ev);

	result->set_accepted(true);
	result->set_next_state(next);
	result->append_event(ev);
	return result;
}
