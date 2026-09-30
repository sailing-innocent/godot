/**************************************************************************/
/*  hex_occupancy.cpp                                                     */
/**************************************************************************/
#include "hex_occupancy.h"

#include "core/object/class_db.h"

static uint64_t _pack_micro(int32_t p_q, int32_t p_r) {
	return ((uint64_t)(uint32_t)p_q << 32) | (uint32_t)p_r;
}

void HexOccupancySnapshot::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_unit", "coord", "side"), &HexOccupancySnapshot::set_unit);
	ClassDB::bind_method(D_METHOD("clear_unit", "coord"), &HexOccupancySnapshot::clear_unit);
	ClassDB::bind_method(D_METHOD("is_unit_at", "coord"), &HexOccupancySnapshot::is_unit_at);
	ClassDB::bind_method(D_METHOD("get_unit_side", "coord"), &HexOccupancySnapshot::get_unit_side);
	ClassDB::bind_method(D_METHOD("get_unit_count"), &HexOccupancySnapshot::get_unit_count);
	ClassDB::bind_method(D_METHOD("clear"), &HexOccupancySnapshot::clear);
	ClassDB::bind_method(D_METHOD("to_dict"), &HexOccupancySnapshot::to_dict);
	ClassDB::bind_static_method("HexOccupancySnapshot", D_METHOD("from_dict", "dict"), &HexOccupancySnapshot::from_dict);
}

void HexOccupancySnapshot::set_unit(const Ref<MicroCoord> &p_coord, int32_t p_side) {
	ERR_FAIL_COND(p_coord.is_null());
	units[_pack_micro(p_coord->get_q(), p_coord->get_r())] = p_side;
}

void HexOccupancySnapshot::clear_unit(const Ref<MicroCoord> &p_coord) {
	ERR_FAIL_COND(p_coord.is_null());
	units.erase(_pack_micro(p_coord->get_q(), p_coord->get_r()));
}

bool HexOccupancySnapshot::is_unit_at(const Ref<MicroCoord> &p_coord) const {
	if (p_coord.is_null()) {
		return false;
	}
	return units.has(_pack_micro(p_coord->get_q(), p_coord->get_r()));
}

int32_t HexOccupancySnapshot::get_unit_side(const Ref<MicroCoord> &p_coord) const {
	if (p_coord.is_null()) {
		return 0;
	}
	const int32_t *s = units.getptr(_pack_micro(p_coord->get_q(), p_coord->get_r()));
	return s ? *s : 0;
}

int HexOccupancySnapshot::get_unit_count() const { return units.size(); }
void HexOccupancySnapshot::clear() { units.clear(); }

Dictionary HexOccupancySnapshot::to_dict() const {
	Dictionary d;
	for (const KeyValue<uint64_t, int32_t> &kv : units) {
		d[(int64_t)kv.key] = kv.value;
	}
	return d;
}

Ref<HexOccupancySnapshot> HexOccupancySnapshot::from_dict(const Dictionary &p_dict) {
	Ref<HexOccupancySnapshot> o;
	o.instantiate();
	const LocalVector<Variant> keys = p_dict.get_key_list();
	for (const Variant &k : keys) {
		o->units[(uint64_t)(int64_t)k] = p_dict[k];
	}
	return o;
}
