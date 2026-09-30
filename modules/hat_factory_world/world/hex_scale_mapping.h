/**************************************************************************/
/*  hex_scale_mapping.h                                                   */
/*  Hat Factory World module — fractal scale mapping (world <-> micro).  */
/*                                                                        */
/*  Single-level rule (architecture §3.1):                                */
/*   - Each detail level declares scale = n (integer >= 2).               */
/*   - World axial cell m = (q, r) maps to the micro anchor anchor(m) =   */
/*     (n*q, n*r).                                                        */
/*   - A micro cell u = (Q, R) belongs to the world cell whose anchor is  */
/*     nearest in the hex metric (discrete Voronoi). Ties are broken by   */
/*     lexicographic (q, r) order, independent of load order.             */
/*   - world_hex_size is fixed; micro_hex_size = world_hex_size / n.      */
/*                                                                        */
/*  Ownership uses exact integer arithmetic (no floating-point drift)     */
/*  and floor division, so negative coordinates behave correctly.         */
/*                                                                        */
/*  v1 supports exactly two levels (world + one detail level). Deeper     */
/*  cumulative products are an explicit extension point: do not hardcode  */
/*  multi-level composition here.                                         */
/**************************************************************************/
#ifndef HEX_SCALE_MAPPING_H
#define HEX_SCALE_MAPPING_H

#include "world_ids.h"

#include "core/io/resource.h"
#include "core/math/vector2.h"
#include "core/math/vector2i.h"
#include "core/object/ref_counted.h"

/**
 * @brief Immutable declaration of one detail-map subdivision level.
 *
 * Fields are fixed by the map author; two descriptors of the same
 * continuous map must agree on scale/origin/orientation (cross-shard rule).
 * Additional fields for future version negotiation (multi-level cumulative
 * products, heterogeneous scale) must be added here, not hardcoded.
 */
class DetailMapDescriptor : public Resource {
	GDCLASS(DetailMapDescriptor, Resource)
	StringName map_id;
	int schema_version = 1;
	int generator_version = 1;
	int scale = 3; // n >= 2
	Vector2i origin; // world cell owning micro (0,0); mapping metadata for v1
	double elevation_step = 1.0; // world meters per micro height level
	StringName orientation = StringName("pointy_top"); // v1 supports pointy_top only

protected:
	static void _bind_methods();

public:
	void set_map_id(const StringName &p_id);
	StringName get_map_id() const;
	void set_schema_version(int p_version);
	int get_schema_version() const;
	void set_generator_version(int p_version);
	int get_generator_version() const;
	void set_scale(int p_scale);
	int get_scale() const;
	void set_origin(const Vector2i &p_origin);
	Vector2i get_origin() const;
	void set_elevation_step(double p_step);
	double get_elevation_step() const;
	void set_orientation(const StringName &p_orientation);
	StringName get_orientation() const;

	Dictionary to_dict() const;
	static Ref<DetailMapDescriptor> from_dict(const Dictionary &p_dict);
	bool equals(const Ref<DetailMapDescriptor> &p_other) const;
	/** Returns an empty string when valid, otherwise the rejection reason. */
	String validate() const;
	String to_string() const;
};

/**
 * @brief Scale conversion between world cells and micro cells.
 *
 * Instance is bound to a DetailMapDescriptor; all conversions carry the
 * scale explicitly through their names and argument/return types.
 */
class HexScaleMapping : public RefCounted {
	GDCLASS(HexScaleMapping, RefCounted)
	Ref<DetailMapDescriptor> descriptor;

protected:
	static void _bind_methods();

public:
	void set_descriptor(const Ref<DetailMapDescriptor> &p_descriptor);
	Ref<DetailMapDescriptor> get_descriptor() const;

	/** anchor(m) = (n*q, n*r) — deterministic, exact. */
	Ref<MicroCoord> world_cell_to_micro_anchor(const Ref<WorldCellCoord> &p_world) const;
	/**
	 * Discrete Voronoi owner of a micro cell. Exact integer arithmetic;
	 * ties resolved lexicographically so any load order yields the same map.
	 */
	Ref<WorldCellCoord> micro_cell_to_world_cell(const Ref<MicroCoord> &p_micro) const;
	/**
	 * Same conversion for C++ hot paths (no refcount churn).
	 * Output world cell is written to r_world_q / r_world_r.
	 */
	bool micro_cell_to_world_cell_i(int32_t p_micro_q, int32_t p_micro_r, int32_t &r_world_q, int32_t &r_world_r) const;

	/** Pointy-top hex center in world units (world_hex_size = center-to-corner). */
	Vector2 world_cell_center(const Ref<WorldCellCoord> &p_world, double p_world_hex_size) const;
	Vector2 micro_cell_center(const Ref<MicroCoord> &p_micro, double p_world_hex_size) const;
	double get_micro_hex_size(double p_world_hex_size) const;

	/** Rejection reason from the descriptor, empty when usable. */
	String validate() const;

	static Ref<HexScaleMapping> create(const Ref<DetailMapDescriptor> &p_descriptor);
};

#endif // HEX_SCALE_MAPPING_H
