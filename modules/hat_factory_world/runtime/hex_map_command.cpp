/**************************************************************************/
/*  hex_map_command.cpp                                                   */
/**************************************************************************/
#include "hex_map_command.h"

#include "core/object/class_db.h"

void HexMapCommand::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_run_id", "id"), &HexMapCommand::set_run_id);
	ClassDB::bind_method(D_METHOD("get_run_id"), &HexMapCommand::get_run_id);
	ClassDB::bind_method(D_METHOD("set_scale", "scale"), &HexMapCommand::set_scale);
	ClassDB::bind_method(D_METHOD("get_scale"), &HexMapCommand::get_scale);
	ClassDB::bind_method(D_METHOD("set_operation", "op"), &HexMapCommand::set_operation);
	ClassDB::bind_method(D_METHOD("get_operation"), &HexMapCommand::get_operation);
	ClassDB::bind_method(D_METHOD("set_coord", "coord"), &HexMapCommand::set_coord);
	ClassDB::bind_method(D_METHOD("get_coord"), &HexMapCommand::get_coord);
	ClassDB::bind_method(D_METHOD("set_direction", "direction"), &HexMapCommand::set_direction);
	ClassDB::bind_method(D_METHOD("get_direction"), &HexMapCommand::get_direction);
	ClassDB::bind_method(D_METHOD("set_source_id", "id"), &HexMapCommand::set_source_id);
	ClassDB::bind_method(D_METHOD("get_source_id"), &HexMapCommand::get_source_id);
	ClassDB::bind_method(D_METHOD("set_expected_version", "version"), &HexMapCommand::set_expected_version);
	ClassDB::bind_method(D_METHOD("get_expected_version"), &HexMapCommand::get_expected_version);
	ClassDB::bind_method(D_METHOD("set_idempotency_id", "id"), &HexMapCommand::set_idempotency_id);
	ClassDB::bind_method(D_METHOD("get_idempotency_id"), &HexMapCommand::get_idempotency_id);
	ClassDB::bind_method(D_METHOD("set_payload", "payload"), &HexMapCommand::set_payload);
	ClassDB::bind_method(D_METHOD("get_payload"), &HexMapCommand::get_payload);
	ClassDB::bind_method(D_METHOD("set_payload_value", "key", "value"), &HexMapCommand::set_payload_value);
	ClassDB::bind_method(D_METHOD("get_payload_value", "key", "default"), &HexMapCommand::get_payload_value, DEFVAL(Variant()));
	ClassDB::bind_method(D_METHOD("to_dict"), &HexMapCommand::to_dict);
	ClassDB::bind_static_method("HexMapCommand", D_METHOD("from_dict", "dict"), &HexMapCommand::from_dict);
	ClassDB::bind_static_method("HexMapCommand", D_METHOD("make", "operation", "coord"), &HexMapCommand::make);
	ClassDB::bind_static_method("HexMapCommand", D_METHOD("set_cell_base", "coord", "terrain_id"), &HexMapCommand::set_cell_base);
	ClassDB::bind_static_method("HexMapCommand", D_METHOD("set_cell_elevation", "coord", "elevation"), &HexMapCommand::set_cell_elevation);
	ClassDB::bind_static_method("HexMapCommand", D_METHOD("set_edge", "coord", "direction", "edge_type_id", "enabled"), &HexMapCommand::set_edge);
	ClassDB::bind_static_method("HexMapCommand", D_METHOD("set_edge_enabled", "coord", "direction", "enabled", "key_id"), &HexMapCommand::set_edge_enabled, DEFVAL(StringName()));
	ClassDB::bind_static_method("HexMapCommand", D_METHOD("damage_edge", "coord", "direction", "amount"), &HexMapCommand::damage_edge);
	ClassDB::bind_static_method("HexMapCommand", D_METHOD("place_object", "coord", "object_id", "feature_kind", "feature_seq"), &HexMapCommand::place_object, DEFVAL(StringName()), DEFVAL((int64_t)0));
	ClassDB::bind_static_method("HexMapCommand", D_METHOD("move_object", "from", "to"), &HexMapCommand::move_object);
	ClassDB::bind_static_method("HexMapCommand", D_METHOD("damage_object", "coord", "amount"), &HexMapCommand::damage_object);
	ClassDB::bind_static_method("HexMapCommand", D_METHOD("destroy_object", "coord"), &HexMapCommand::destroy_object);
	ClassDB::bind_static_method("HexMapCommand", D_METHOD("apply_area_effect", "coord", "effect_id"), &HexMapCommand::apply_area_effect);
	ClassDB::bind_static_method("HexMapCommand", D_METHOD("remove_area_effect", "coord", "effect_id"), &HexMapCommand::remove_area_effect);
}

void HexMapCommand::set_run_id(int64_t p_id) { run_id = p_id; }
int64_t HexMapCommand::get_run_id() const { return run_id; }
void HexMapCommand::set_scale(const StringName &p_scale) { scale = p_scale; }
StringName HexMapCommand::get_scale() const { return scale; }
void HexMapCommand::set_operation(const StringName &p_op) { operation = p_op; }
StringName HexMapCommand::get_operation() const { return operation; }
void HexMapCommand::set_coord(const Ref<MicroCoord> &p_coord) { coord = p_coord; }
Ref<MicroCoord> HexMapCommand::get_coord() const { return coord; }
void HexMapCommand::set_direction(int p_direction) { direction = p_direction; }
int HexMapCommand::get_direction() const { return direction; }
void HexMapCommand::set_source_id(const StringName &p_id) { source_id = p_id; }
StringName HexMapCommand::get_source_id() const { return source_id; }
void HexMapCommand::set_expected_version(int64_t p_version) { expected_version = p_version; }
int64_t HexMapCommand::get_expected_version() const { return expected_version; }
void HexMapCommand::set_idempotency_id(const StringName &p_id) { idempotency_id = p_id; }
StringName HexMapCommand::get_idempotency_id() const { return idempotency_id; }
void HexMapCommand::set_payload(const Dictionary &p_payload) { payload = p_payload; }
Dictionary HexMapCommand::get_payload() const { return payload; }
void HexMapCommand::set_payload_value(const StringName &p_key, const Variant &p_value) { payload[p_key] = p_value; }
Variant HexMapCommand::get_payload_value(const StringName &p_key, const Variant &p_default) const { return payload.get(p_key, p_default); }

Dictionary HexMapCommand::to_dict() const {
	Dictionary d;
	d[StringName("run_id")] = run_id;
	d[StringName("scale")] = scale;
	d[StringName("operation")] = operation;
	d[StringName("coord")] = coord.is_valid() ? coord->to_dict() : Dictionary();
	d[StringName("direction")] = direction;
	d[StringName("source_id")] = source_id;
	d[StringName("expected_version")] = expected_version;
	d[StringName("idempotency_id")] = idempotency_id;
	d[StringName("payload")] = payload;
	return d;
}

Ref<HexMapCommand> HexMapCommand::from_dict(const Dictionary &p_dict) {
	Ref<HexMapCommand> c = make(
			p_dict.get(StringName("operation"), StringName()),
			MicroCoord::from_dict(p_dict.get(StringName("coord"), Dictionary())));
	c->set_run_id(p_dict.get(StringName("run_id"), (int64_t)0));
	c->set_scale(p_dict.get(StringName("scale"), StringName("micro")));
	c->set_direction(p_dict.get(StringName("direction"), -1));
	c->set_source_id(p_dict.get(StringName("source_id"), StringName()));
	c->set_expected_version(p_dict.get(StringName("expected_version"), (int64_t)0));
	c->set_idempotency_id(p_dict.get(StringName("idempotency_id"), StringName()));
	c->set_payload(p_dict.get(StringName("payload"), Dictionary()));
	return c;
}

Ref<HexMapCommand> HexMapCommand::make(const StringName &p_operation, const Ref<MicroCoord> &p_coord) {
	Ref<HexMapCommand> c;
	c.instantiate();
	c->set_operation(p_operation);
	c->set_coord(p_coord);
	return c;
}

Ref<HexMapCommand> HexMapCommand::set_cell_base(const Ref<MicroCoord> &p_coord, const StringName &p_terrain_id) {
	Ref<HexMapCommand> c = make(StringName("set_cell_base"), p_coord);
	c->set_payload_value(StringName("terrain_id"), p_terrain_id);
	return c;
}

Ref<HexMapCommand> HexMapCommand::set_cell_elevation(const Ref<MicroCoord> &p_coord, int p_elevation) {
	Ref<HexMapCommand> c = make(StringName("set_cell_elevation"), p_coord);
	c->set_payload_value(StringName("elevation"), p_elevation);
	return c;
}

Ref<HexMapCommand> HexMapCommand::set_edge(const Ref<MicroCoord> &p_coord, int p_direction, const StringName &p_edge_type_id, bool p_enabled) {
	Ref<HexMapCommand> c = make(StringName("set_edge"), p_coord);
	c->set_direction(p_direction);
	c->set_payload_value(StringName("edge_type_id"), p_edge_type_id);
	c->set_payload_value(StringName("enabled"), p_enabled);
	return c;
}

Ref<HexMapCommand> HexMapCommand::set_edge_enabled(const Ref<MicroCoord> &p_coord, int p_direction, bool p_enabled, const StringName &p_key_id) {
	Ref<HexMapCommand> c = make(StringName("set_edge_enabled"), p_coord);
	c->set_direction(p_direction);
	c->set_payload_value(StringName("enabled"), p_enabled);
	if (p_key_id != StringName()) {
		c->set_payload_value(StringName("key_id"), p_key_id);
	}
	return c;
}

Ref<HexMapCommand> HexMapCommand::damage_edge(const Ref<MicroCoord> &p_coord, int p_direction, int p_amount) {
	Ref<HexMapCommand> c = make(StringName("damage_edge"), p_coord);
	c->set_direction(p_direction);
	c->set_payload_value(StringName("amount"), p_amount);
	return c;
}

Ref<HexMapCommand> HexMapCommand::place_object(const Ref<MicroCoord> &p_coord, const StringName &p_object_id, const StringName &p_feature_kind, int64_t p_feature_seq) {
	Ref<HexMapCommand> c = make(StringName("place_object"), p_coord);
	c->set_payload_value(StringName("object_id"), p_object_id);
	if (p_feature_kind != StringName()) {
		c->set_payload_value(StringName("feature_kind"), p_feature_kind);
		c->set_payload_value(StringName("feature_seq"), p_feature_seq);
	}
	return c;
}

Ref<HexMapCommand> HexMapCommand::move_object(const Ref<MicroCoord> &p_from, const Ref<MicroCoord> &p_to) {
	Ref<HexMapCommand> c = make(StringName("move_object"), p_from);
	c->set_payload_value(StringName("to_coord"), p_to->to_dict());
	return c;
}

Ref<HexMapCommand> HexMapCommand::damage_object(const Ref<MicroCoord> &p_coord, int p_amount) {
	Ref<HexMapCommand> c = make(StringName("damage_object"), p_coord);
	c->set_payload_value(StringName("amount"), p_amount);
	return c;
}

Ref<HexMapCommand> HexMapCommand::destroy_object(const Ref<MicroCoord> &p_coord) {
	return make(StringName("destroy_object"), p_coord);
}

Ref<HexMapCommand> HexMapCommand::apply_area_effect(const Ref<MicroCoord> &p_coord, const StringName &p_effect_id) {
	Ref<HexMapCommand> c = make(StringName("apply_area_effect"), p_coord);
	c->set_payload_value(StringName("effect_id"), p_effect_id);
	return c;
}

Ref<HexMapCommand> HexMapCommand::remove_area_effect(const Ref<MicroCoord> &p_coord, const StringName &p_effect_id) {
	Ref<HexMapCommand> c = make(StringName("remove_area_effect"), p_coord);
	c->set_payload_value(StringName("effect_id"), p_effect_id);
	return c;
}
