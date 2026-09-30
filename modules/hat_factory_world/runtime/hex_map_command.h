/**************************************************************************/
/*  hex_map_command.h                                                     */
/*  Hat Factory World module — structured map mutation command.          */
/*                                                                        */
/*  Invariant 3: every state change flows as MapCommand -> resolver       */
/*  validation -> COW -> events. Components outside the resolver never    */
/*  receive writable pointers to authoritative state.                     */
/**************************************************************************/
#ifndef HEX_MAP_COMMAND_H
#define HEX_MAP_COMMAND_H

#include "../world/world_ids.h"

#include "core/object/ref_counted.h"
#include "core/string/string_name.h"
#include "core/variant/dictionary.h"

/**
 * @brief Structured mutation command targeting one micro cell/edge/object.
 *
 * v1 operation set:
 *   set_cell_base, set_cell_elevation,
 *   set_edge, set_edge_enabled, damage_edge,
 *   place_object, move_object, damage_object, destroy_object,
 *   apply_area_effect, remove_area_effect,
 *   interact_cell, interact_object,
 *   submit_env_event (Phase 8 hook)
 */
class HexMapCommand : public RefCounted {
	GDCLASS(HexMapCommand, RefCounted)
	int64_t run_id = 0;
	StringName scale = StringName("micro"); // "world" | "micro" (v1 ops are micro; world = portal/gate ops)
	StringName operation;
	Ref<MicroCoord> coord;
	int direction = -1; // edge direction 0..5 when targeting an edge
	StringName source_id; // owning entity (actor id) for ownership/capability checks
	int64_t expected_version = 0; // snapshot version the command was built against
	StringName idempotency_id; // journal dedup key; empty = none
	Dictionary payload; // op-specific data (terrain id, object id, key items, amounts...)

protected:
	static void _bind_methods();

public:
	void set_run_id(int64_t p_id);
	int64_t get_run_id() const;
	void set_scale(const StringName &p_scale);
	StringName get_scale() const;
	void set_operation(const StringName &p_op);
	StringName get_operation() const;
	void set_coord(const Ref<MicroCoord> &p_coord);
	Ref<MicroCoord> get_coord() const;
	void set_direction(int p_direction);
	int get_direction() const;
	void set_source_id(const StringName &p_id);
	StringName get_source_id() const;
	void set_expected_version(int64_t p_version);
	int64_t get_expected_version() const;
	void set_idempotency_id(const StringName &p_id);
	StringName get_idempotency_id() const;
	void set_payload(const Dictionary &p_payload);
	Dictionary get_payload() const;
	void set_payload_value(const StringName &p_key, const Variant &p_value);
	Variant get_payload_value(const StringName &p_key, const Variant &p_default = Variant()) const;

	Dictionary to_dict() const;
	static Ref<HexMapCommand> from_dict(const Dictionary &p_dict);

	// Factories for the v1 operation set.
	static Ref<HexMapCommand> make(const StringName &p_operation, const Ref<MicroCoord> &p_coord);
	static Ref<HexMapCommand> set_cell_base(const Ref<MicroCoord> &p_coord, const StringName &p_terrain_id);
	static Ref<HexMapCommand> set_cell_elevation(const Ref<MicroCoord> &p_coord, int p_elevation);
	static Ref<HexMapCommand> set_edge(const Ref<MicroCoord> &p_coord, int p_direction, const StringName &p_edge_type_id, bool p_enabled);
	static Ref<HexMapCommand> set_edge_enabled(const Ref<MicroCoord> &p_coord, int p_direction, bool p_enabled, const StringName &p_key_id = StringName());
	static Ref<HexMapCommand> damage_edge(const Ref<MicroCoord> &p_coord, int p_direction, int p_amount);
	static Ref<HexMapCommand> place_object(const Ref<MicroCoord> &p_coord, const StringName &p_object_id, const StringName &p_feature_kind = StringName(), int64_t p_feature_seq = 0);
	static Ref<HexMapCommand> move_object(const Ref<MicroCoord> &p_from, const Ref<MicroCoord> &p_to);
	static Ref<HexMapCommand> damage_object(const Ref<MicroCoord> &p_coord, int p_amount);
	static Ref<HexMapCommand> destroy_object(const Ref<MicroCoord> &p_coord);
	static Ref<HexMapCommand> apply_area_effect(const Ref<MicroCoord> &p_coord, const StringName &p_effect_id);
	static Ref<HexMapCommand> remove_area_effect(const Ref<MicroCoord> &p_coord, const StringName &p_effect_id);
};

#endif // HEX_MAP_COMMAND_H
