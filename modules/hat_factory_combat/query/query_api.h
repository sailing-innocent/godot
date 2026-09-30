#ifndef QUERY_API_H
#define QUERY_API_H

#include "core/math/vector2i.h"
#include "core/object/ref_counted.h"
#include "core/variant/typed_array.h"

#include "modules/hat_factory_combat/state/battle_state.h"

class CombatEntity;

class QueryAPI : public RefCounted {
	GDCLASS(QueryAPI, RefCounted)
	Ref<BattleState> state;
	// DEPRECATED (dual-map v1, 2026-10): combat map queries are being
	// re-pointed at hat_factory_world's HexSpatialQuery (single source of
	// spatial truth, shared by travel and battle). The signature stays as an
	// adapter during the migration window; new code must call HexSpatialQuery.
protected:
	static void _bind_methods();
public:
	static void _bind_methods();

public:
	QueryAPI() = default;
	explicit QueryAPI(const Ref<BattleState> &p_state) { state = p_state; }

	void set_state(const Ref<BattleState> &p_value) { state = p_value; }
	Ref<BattleState> get_state() const { return state; }

	bool has_entity(int p_id) const;
	Ref<CombatEntity> get_entity(int p_id) const;
	TypedArray<int> get_entities_by_owner(int p_owner) const;
	Ref<CombatEntity> get_entity_at(const Vector2i &p_coord) const;
	TypedArray<Vector2i> get_reachable_cells(int p_actor_id, int p_range) const;
	bool can_move_to(int p_actor_id, const Vector2i &p_coord) const;

	static int hex_distance(const Vector2i &p_a, const Vector2i &p_b);
	static TypedArray<Vector2i> get_neighbors(const Vector2i &p_coord);
};

#endif // QUERY_API_H
