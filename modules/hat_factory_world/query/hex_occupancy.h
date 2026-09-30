/**************************************************************************/
/*  hex_occupancy.h                                                       */
/*  Hat Factory World module — occupancy layers (MAP-14).                 */
/*                                                                        */
/*  Layered occupancy: units (per side), placed objects come from the map  */
/*  snapshot itself; traversal rules (friend crossable vs blocked) are     */
/*  decided by the movement profile.                                       */
/**************************************************************************/
#ifndef HEX_OCCUPANCY_H
#define HEX_OCCUPANCY_H

#include "../world/world_ids.h"

#include "core/object/ref_counted.h"
#include "core/templates/hash_map.h"

class HexOccupancySnapshot : public RefCounted {
	GDCLASS(HexOccupancySnapshot, RefCounted)
	// packed micro coord -> side marker (>0 = side id, <0 = blocking neutral)
	HashMap<uint64_t, int32_t> units;

protected:
	static void _bind_methods();

public:
	void set_unit(const Ref<MicroCoord> &p_coord, int32_t p_side);
	void clear_unit(const Ref<MicroCoord> &p_coord);
	bool is_unit_at(const Ref<MicroCoord> &p_coord) const;
	int32_t get_unit_side(const Ref<MicroCoord> &p_coord) const;
	int get_unit_count() const;
	void clear();

	Dictionary to_dict() const;
	static Ref<HexOccupancySnapshot> from_dict(const Dictionary &p_dict);
};

#endif // HEX_OCCUPANCY_H
