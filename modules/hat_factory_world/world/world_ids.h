/**************************************************************************/
/*  world_ids.h                                                           */
/*  Hat Factory World module — strongly typed identity layer.            */
/*                                                                        */
/*  Every public API in hat_factory_world carries scale explicitly        */
/*  (WorldCellCoord vs MicroCoord); bare Vector2i must not mix scales.    */
/*  All identity types support to_dict/from_dict, hash() and compare/     */
/*  equals so they can act as dictionary keys and survive serialization.  */
/**************************************************************************/
#ifndef WORLD_IDS_H
#define WORLD_IDS_H

#include "core/object/ref_counted.h"
#include "core/string/string_name.h"
#include "core/variant/dictionary.h"
#include "core/variant/typed_array.h"

/**
 * @brief Identifies a continuous map instance (e.g. "demo_world").
 * Stable across shards, streaming and save/load.
 */
class MapId : public RefCounted {
	GDCLASS(MapId, RefCounted)
	StringName value;

protected:
	static void _bind_methods();

public:
	void set_value(const StringName &p_value);
	StringName get_value() const;
	bool is_valid() const;
	Dictionary to_dict() const;
	static Ref<MapId> from_dict(const Dictionary &p_dict);
	bool equals(const Ref<MapId> &p_other) const;
	int compare_to(const Ref<MapId> &p_other) const;
	uint32_t hash() const;
	String to_string() const;
};

/**
 * @brief Version of an immutable template: schema + generator versions.
 * Incompatible schema versions must be rejected on load (no silent replay).
 */
class TemplateVersion : public RefCounted {
	GDCLASS(TemplateVersion, RefCounted)
	int schema_version = 1;
	int generator_version = 1;

protected:
	static void _bind_methods();

public:
	void set_schema_version(int p_version);
	int get_schema_version() const;
	void set_generator_version(int p_version);
	int get_generator_version() const;
	Dictionary to_dict() const;
	static Ref<TemplateVersion> from_dict(const Dictionary &p_dict);
	bool equals(const Ref<TemplateVersion> &p_other) const;
	int compare_to(const Ref<TemplateVersion> &p_other) const;
	/** True when schema matches; generator may differ (regeneration allowed). */
	bool is_compatible_with(const Ref<TemplateVersion> &p_other) const;
	String to_string() const;
};

/**
 * @brief Axial coordinate of a macro/world cell (Scale::WORLD).
 */
class WorldCellCoord : public RefCounted {
	GDCLASS(WorldCellCoord, RefCounted)
	int32_t q = 0;
	int32_t r = 0;

protected:
	static void _bind_methods();

public:
	void set_q(int32_t p_q);
	int32_t get_q() const;
	void set_r(int32_t p_r);
	int32_t get_r() const;
	void set_qr(int32_t p_q, int32_t p_r);
	bool is_valid() const;
	Dictionary to_dict() const;
	static Ref<WorldCellCoord> from_dict(const Dictionary &p_dict);
	static Ref<WorldCellCoord> make(int32_t p_q, int32_t p_r);
	bool equals(const Ref<WorldCellCoord> &p_other) const;
	/** Lexicographic (q, then r) three-way comparison. */
	int compare_to(const Ref<WorldCellCoord> &p_other) const;
	uint32_t hash() const;
	/** Hex (cube) distance to another world cell. */
	int distance_to(const Ref<WorldCellCoord> &p_other) const;
	TypedArray<WorldCellCoord> neighbors() const;
	String to_string() const;
};

/**
 * @brief Axial coordinate of a micro/battle cell (Scale::MICRO).
 * detail_level starts at 1 for the first subdivision below world scale.
 */
class MicroCoord : public RefCounted {
	GDCLASS(MicroCoord, RefCounted)
	int32_t q = 0;
	int32_t r = 0;
	int detail_level = 1;

protected:
	static void _bind_methods();

public:
	void set_q(int32_t p_q);
	int32_t get_q() const;
	void set_r(int32_t p_r);
	int32_t get_r() const;
	void set_qr(int32_t p_q, int32_t p_r);
	void set_detail_level(int p_level);
	int get_detail_level() const;
	bool is_valid() const;
	Dictionary to_dict() const;
	static Ref<MicroCoord> from_dict(const Dictionary &p_dict);
	static Ref<MicroCoord> make(int32_t p_q, int32_t p_r, int p_detail_level = 1);
	bool equals(const Ref<MicroCoord> &p_other) const;
	/** Lexicographic (q, then r, then detail_level) three-way comparison. */
	int compare_to(const Ref<MicroCoord> &p_other) const;
	uint32_t hash() const;
	String to_string() const;
};

/**
 * @brief Stable identity of a map feature (gate, bridge, reef, trigger...).
 * A feature keeps one owner ID across cells, edges, shards and streaming
 * reloads; it never receives a new ID on stream-in.
 */
class FeatureId : public RefCounted {
	GDCLASS(FeatureId, RefCounted)
	StringName kind; // "gate" / "bridge" / "reef" / "trigger" / ...
	int64_t seq = 0; // stable sequence, unique per (map_id, kind)

protected:
	static void _bind_methods();

public:
	void set_kind(const StringName &p_kind);
	StringName get_kind() const;
	void set_seq(int64_t p_seq);
	int64_t get_seq() const;
	bool is_valid() const;
	Dictionary to_dict() const;
	static Ref<FeatureId> from_dict(const Dictionary &p_dict);
	static Ref<FeatureId> make(const StringName &p_kind, int64_t p_seq);
	bool equals(const Ref<FeatureId> &p_other) const;
	int compare_to(const Ref<FeatureId> &p_other) const;
	uint32_t hash() const;
	String to_string() const;
};

/**
 * @brief Normalized undirected edge between two adjacent world cells.
 * Endpoints are stored lexicographically (q, then r) so the key is
 * independent of construction/load order.
 */
class EdgeKey : public RefCounted {
	GDCLASS(EdgeKey, RefCounted)
	Ref<WorldCellCoord> endpoint_a;
	Ref<WorldCellCoord> endpoint_b;

protected:
	static void _bind_methods();

public:
	void set_endpoint_a(const Ref<WorldCellCoord> &p_coord);
	Ref<WorldCellCoord> get_endpoint_a() const;
	void set_endpoint_b(const Ref<WorldCellCoord> &p_coord);
	Ref<WorldCellCoord> get_endpoint_b() const;
	bool is_valid() const;
	Dictionary to_dict() const;
	static Ref<EdgeKey> from_dict(const Dictionary &p_dict);
	/** Normalizing factory: endpoints reordered lexicographically. */
	static Ref<EdgeKey> make(const Ref<WorldCellCoord> &p_a, const Ref<WorldCellCoord> &p_b);
	bool equals(const Ref<EdgeKey> &p_other) const;
	int compare_to(const Ref<EdgeKey> &p_other) const;
	uint32_t hash() const;
	String to_string() const;
};

#endif // WORLD_IDS_H
