/**************************************************************************/
/*  hex_scale_mapping.cpp                                                 */
/**************************************************************************/
#include "hex_scale_mapping.h"

#include "core/math/math_funcs.h"
#include "core/object/class_db.h"
#include "core/string/ustring.h"

/**************************************************************************/
/* DetailMapDescriptor                                                    */
/**************************************************************************/

void DetailMapDescriptor::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_map_id", "id"), &DetailMapDescriptor::set_map_id);
	ClassDB::bind_method(D_METHOD("get_map_id"), &DetailMapDescriptor::get_map_id);
	ClassDB::bind_method(D_METHOD("set_schema_version", "version"), &DetailMapDescriptor::set_schema_version);
	ClassDB::bind_method(D_METHOD("get_schema_version"), &DetailMapDescriptor::get_schema_version);
	ClassDB::bind_method(D_METHOD("set_generator_version", "version"), &DetailMapDescriptor::set_generator_version);
	ClassDB::bind_method(D_METHOD("get_generator_version"), &DetailMapDescriptor::get_generator_version);
	ClassDB::bind_method(D_METHOD("set_scale", "scale"), &DetailMapDescriptor::set_scale);
	ClassDB::bind_method(D_METHOD("get_scale"), &DetailMapDescriptor::get_scale);
	ClassDB::bind_method(D_METHOD("set_origin", "origin"), &DetailMapDescriptor::set_origin);
	ClassDB::bind_method(D_METHOD("get_origin"), &DetailMapDescriptor::get_origin);
	ClassDB::bind_method(D_METHOD("set_elevation_step", "step"), &DetailMapDescriptor::set_elevation_step);
	ClassDB::bind_method(D_METHOD("get_elevation_step"), &DetailMapDescriptor::get_elevation_step);
	ClassDB::bind_method(D_METHOD("set_orientation", "orientation"), &DetailMapDescriptor::set_orientation);
	ClassDB::bind_method(D_METHOD("get_orientation"), &DetailMapDescriptor::get_orientation);
	ClassDB::bind_method(D_METHOD("to_dict"), &DetailMapDescriptor::to_dict);
	ClassDB::bind_static_method("DetailMapDescriptor", D_METHOD("from_dict", "dict"), &DetailMapDescriptor::from_dict);
	ClassDB::bind_method(D_METHOD("equals", "other"), &DetailMapDescriptor::equals);
	ClassDB::bind_method(D_METHOD("validate"), &DetailMapDescriptor::validate);
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "map_id"), "set_map_id", "get_map_id");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "schema_version"), "set_schema_version", "get_schema_version");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "generator_version"), "set_generator_version", "get_generator_version");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "scale"), "set_scale", "get_scale");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2I, "origin"), "set_origin", "get_origin");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "elevation_step"), "set_elevation_step", "get_elevation_step");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "orientation"), "set_orientation", "get_orientation");
}

void DetailMapDescriptor::set_map_id(const StringName &p_id) { map_id = p_id; }
StringName DetailMapDescriptor::get_map_id() const { return map_id; }
void DetailMapDescriptor::set_schema_version(int p_version) { schema_version = p_version; }
int DetailMapDescriptor::get_schema_version() const { return schema_version; }
void DetailMapDescriptor::set_generator_version(int p_version) { generator_version = p_version; }
int DetailMapDescriptor::get_generator_version() const { return generator_version; }
void DetailMapDescriptor::set_scale(int p_scale) { scale = p_scale; }
int DetailMapDescriptor::get_scale() const { return scale; }
void DetailMapDescriptor::set_origin(const Vector2i &p_origin) { origin = p_origin; }
Vector2i DetailMapDescriptor::get_origin() const { return origin; }
void DetailMapDescriptor::set_elevation_step(double p_step) { elevation_step = p_step; }
double DetailMapDescriptor::get_elevation_step() const { return elevation_step; }
void DetailMapDescriptor::set_orientation(const StringName &p_orientation) { orientation = p_orientation; }
StringName DetailMapDescriptor::get_orientation() const { return orientation; }

Dictionary DetailMapDescriptor::to_dict() const {
	Dictionary d;
	d[StringName("map_id")] = map_id;
	d[StringName("schema_version")] = schema_version;
	d[StringName("generator_version")] = generator_version;
	d[StringName("scale")] = scale;
	d[StringName("origin")] = origin;
	d[StringName("elevation_step")] = elevation_step;
	d[StringName("orientation")] = orientation;
	return d;
}

Ref<DetailMapDescriptor> DetailMapDescriptor::from_dict(const Dictionary &p_dict) {
	Ref<DetailMapDescriptor> d;
	d.instantiate();
	d->set_map_id(p_dict.get(StringName("map_id"), StringName()));
	d->set_schema_version(p_dict.get(StringName("schema_version"), 1));
	d->set_generator_version(p_dict.get(StringName("generator_version"), 1));
	d->set_scale(p_dict.get(StringName("scale"), 3));
	d->set_origin(p_dict.get(StringName("origin"), Vector2i()));
	d->set_elevation_step(p_dict.get(StringName("elevation_step"), 1.0));
	d->set_orientation(p_dict.get(StringName("orientation"), StringName("pointy_top")));
	return d;
}

bool DetailMapDescriptor::equals(const Ref<DetailMapDescriptor> &p_other) const {
	if (p_other.is_null()) {
		return false;
	}
	return map_id == p_other->map_id &&
			schema_version == p_other->schema_version &&
			generator_version == p_other->generator_version &&
			scale == p_other->scale &&
			origin == p_other->origin &&
			elevation_step == p_other->elevation_step &&
			orientation == p_other->orientation;
}

String DetailMapDescriptor::validate() const {
	if (scale < 2) {
		return "scale must be an integer >= 2 (got " + itos(scale) + ")";
	}
	if (orientation != StringName("pointy_top")) {
		return "v1 supports orientation 'pointy_top' only (got '" + String(orientation) + "')";
	}
	if (elevation_step <= 0.0) {
		return "elevation_step must be > 0";
	}
	if (schema_version < 1 || generator_version < 1) {
		return "schema/generator versions must be >= 1";
	}
	return String();
}

String DetailMapDescriptor::to_string() const {
	return "DetailMapDescriptor(" + String(map_id) + " scale=" + itos(scale) + " v" + itos(schema_version) + "." + itos(generator_version) + ")";
}

/**************************************************************************/
/* HexScaleMapping                                                        */
/**************************************************************************/

void HexScaleMapping::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_descriptor", "descriptor"), &HexScaleMapping::set_descriptor);
	ClassDB::bind_method(D_METHOD("get_descriptor"), &HexScaleMapping::get_descriptor);
	ClassDB::bind_method(D_METHOD("world_cell_to_micro_anchor", "world"), &HexScaleMapping::world_cell_to_micro_anchor);
	ClassDB::bind_method(D_METHOD("micro_cell_to_world_cell", "micro"), &HexScaleMapping::micro_cell_to_world_cell);
	ClassDB::bind_method(D_METHOD("world_cell_center", "world", "world_hex_size"), &HexScaleMapping::world_cell_center);
	ClassDB::bind_method(D_METHOD("micro_cell_center", "micro", "world_hex_size"), &HexScaleMapping::micro_cell_center);
	ClassDB::bind_method(D_METHOD("get_micro_hex_size", "world_hex_size"), &HexScaleMapping::get_micro_hex_size);
	ClassDB::bind_method(D_METHOD("validate"), &HexScaleMapping::validate);
	ClassDB::bind_static_method("HexScaleMapping", D_METHOD("create", "descriptor"), &HexScaleMapping::create);
}

void HexScaleMapping::set_descriptor(const Ref<DetailMapDescriptor> &p_descriptor) { descriptor = p_descriptor; }
Ref<DetailMapDescriptor> HexScaleMapping::get_descriptor() const { return descriptor; }

Ref<HexScaleMapping> HexScaleMapping::create(const Ref<DetailMapDescriptor> &p_descriptor) {
	Ref<HexScaleMapping> m;
	m.instantiate();
	m->set_descriptor(p_descriptor);
	return m;
}

/** Floor division for positive divisor; correct for negative dividends. */
static int32_t _floordiv_pos(int32_t a, int32_t n) {
	if (a >= 0) {
		return a / n;
	}
	return (int32_t)(-((-(int64_t)a + (int64_t)n - 1) / n));
}

Ref<MicroCoord> HexScaleMapping::world_cell_to_micro_anchor(const Ref<WorldCellCoord> &p_world) const {
	ERR_FAIL_COND_V(descriptor.is_null(), Ref<MicroCoord>());
	ERR_FAIL_COND_V(p_world.is_null(), Ref<MicroCoord>());
	const int32_t n = descriptor->get_scale();
	return MicroCoord::make(p_world->get_q() * n, p_world->get_r() * n, 1);
}

bool HexScaleMapping::micro_cell_to_world_cell_i(int32_t p_micro_q, int32_t p_micro_r, int32_t &r_world_q, int32_t &r_world_r) const {
	ERR_FAIL_COND_V(descriptor.is_null(), false);
	const int32_t n = (int32_t)descriptor->get_scale();
	ERR_FAIL_COND_V(n < 2, false);

	// Exact integer Voronoi: minimize f = A^2 + A*B + B^2 with
	// A = n*q - Q, B = n*r - R over the provably sufficient window
	// [floor-1, floor+2]^2. Ties break lexicographically (q, then r),
	// so the result is independent of construction/load order.
	const int32_t q0 = _floordiv_pos(p_micro_q, n);
	const int32_t r0 = _floordiv_pos(p_micro_r, n);
	int64_t best = INT64_MAX;
	int32_t best_q = q0;
	int32_t best_r = r0;
	for (int32_t q = q0 - 1; q <= q0 + 2; q++) {
		for (int32_t r = r0 - 1; r <= r0 + 2; r++) {
			const int64_t a = (int64_t)n * q - p_micro_q;
			const int64_t b = (int64_t)n * r - p_micro_r;
			const int64_t f = a * a + a * b + b * b;
			if (f < best || (f == best && (q < best_q || (q == best_q && r < best_r)))) {
				best = f;
				best_q = q;
				best_r = r;
			}
		}
	}
	r_world_q = best_q;
	r_world_r = best_r;
	return true;
}

Ref<WorldCellCoord> HexScaleMapping::micro_cell_to_world_cell(const Ref<MicroCoord> &p_micro) const {
	ERR_FAIL_COND_V(p_micro.is_null(), Ref<WorldCellCoord>());
	int32_t wq = 0;
	int32_t wr = 0;
	ERR_FAIL_COND_V(!micro_cell_to_world_cell_i(p_micro->get_q(), p_micro->get_r(), wq, wr), Ref<WorldCellCoord>());
	return WorldCellCoord::make(wq, wr);
}

Vector2 HexScaleMapping::world_cell_center(const Ref<WorldCellCoord> &p_world, double p_world_hex_size) const {
	ERR_FAIL_COND_V(p_world.is_null(), Vector2());
	const double sq3 = Math::sqrt(3.0);
	return Vector2(
			p_world_hex_size * sq3 * (p_world->get_q() + p_world->get_r() * 0.5),
			p_world_hex_size * 1.5 * p_world->get_r());
}

Vector2 HexScaleMapping::micro_cell_center(const Ref<MicroCoord> &p_micro, double p_world_hex_size) const {
	ERR_FAIL_COND_V(p_micro.is_null(), Vector2());
	ERR_FAIL_COND_V(descriptor.is_null(), Vector2());
	const double micro_size = get_micro_hex_size(p_world_hex_size);
	const double sq3 = Math::sqrt(3.0);
	return Vector2(
			micro_size * sq3 * (p_micro->get_q() + p_micro->get_r() * 0.5),
			micro_size * 1.5 * p_micro->get_r());
}

double HexScaleMapping::get_micro_hex_size(double p_world_hex_size) const {
	ERR_FAIL_COND_V(descriptor.is_null(), 0.0);
	return p_world_hex_size / (double)descriptor->get_scale();
}

String HexScaleMapping::validate() const {
	if (descriptor.is_null()) {
		return "missing DetailMapDescriptor";
	}
	return descriptor->validate();
}
