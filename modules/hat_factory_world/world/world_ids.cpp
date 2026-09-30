/**************************************************************************/
/*  world_ids.cpp                                                         */
/**************************************************************************/
#include "world_ids.h"

#include "core/object/class_db.h"
#include "core/string/ustring.h"
#include "core/templates/hashfuncs.h"

/**************************************************************************/
/* MapId                                                                  */
/**************************************************************************/

void MapId::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_value", "value"), &MapId::set_value);
	ClassDB::bind_method(D_METHOD("get_value"), &MapId::get_value);
	ClassDB::bind_method(D_METHOD("is_valid"), &MapId::is_valid);
	ClassDB::bind_method(D_METHOD("to_dict"), &MapId::to_dict);
	ClassDB::bind_static_method("MapId", D_METHOD("from_dict", "dict"), &MapId::from_dict);
	ClassDB::bind_method(D_METHOD("equals", "other"), &MapId::equals);
	ClassDB::bind_method(D_METHOD("compare_to", "other"), &MapId::compare_to);
	ClassDB::bind_method(D_METHOD("hash"), &MapId::hash);
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "value"), "set_value", "get_value");
}

void MapId::set_value(const StringName &p_value) { value = p_value; }
StringName MapId::get_value() const { return value; }
bool MapId::is_valid() const { return value != StringName(); }

Dictionary MapId::to_dict() const {
	Dictionary d;
	d[StringName("value")] = value;
	return d;
}

Ref<MapId> MapId::from_dict(const Dictionary &p_dict) {
	Ref<MapId> id;
	id.instantiate();
	id->set_value(p_dict.get(StringName("value"), StringName()));
	return id;
}

bool MapId::equals(const Ref<MapId> &p_other) const {
	return p_other.is_valid() && value == p_other->value;
}

int MapId::compare_to(const Ref<MapId> &p_other) const {
	if (p_other.is_null()) {
		return 1;
	}
	return String(value).casecmp_to(String(p_other->value));
}

uint32_t MapId::hash() const {
	return value.hash();
}

String MapId::to_string() const {
	return String(value);
}

/**************************************************************************/
/* TemplateVersion                                                        */
/**************************************************************************/

void TemplateVersion::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_schema_version", "version"), &TemplateVersion::set_schema_version);
	ClassDB::bind_method(D_METHOD("get_schema_version"), &TemplateVersion::get_schema_version);
	ClassDB::bind_method(D_METHOD("set_generator_version", "version"), &TemplateVersion::set_generator_version);
	ClassDB::bind_method(D_METHOD("get_generator_version"), &TemplateVersion::get_generator_version);
	ClassDB::bind_method(D_METHOD("to_dict"), &TemplateVersion::to_dict);
	ClassDB::bind_static_method("TemplateVersion", D_METHOD("from_dict", "dict"), &TemplateVersion::from_dict);
	ClassDB::bind_method(D_METHOD("equals", "other"), &TemplateVersion::equals);
	ClassDB::bind_method(D_METHOD("compare_to", "other"), &TemplateVersion::compare_to);
	ClassDB::bind_method(D_METHOD("is_compatible_with", "other"), &TemplateVersion::is_compatible_with);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "schema_version"), "set_schema_version", "get_schema_version");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "generator_version"), "set_generator_version", "get_generator_version");
}

void TemplateVersion::set_schema_version(int p_version) { schema_version = p_version; }
int TemplateVersion::get_schema_version() const { return schema_version; }
void TemplateVersion::set_generator_version(int p_version) { generator_version = p_version; }
int TemplateVersion::get_generator_version() const { return generator_version; }

Dictionary TemplateVersion::to_dict() const {
	Dictionary d;
	d[StringName("schema_version")] = schema_version;
	d[StringName("generator_version")] = generator_version;
	return d;
}

Ref<TemplateVersion> TemplateVersion::from_dict(const Dictionary &p_dict) {
	Ref<TemplateVersion> v;
	v.instantiate();
	v->set_schema_version(p_dict.get(StringName("schema_version"), 1));
	v->set_generator_version(p_dict.get(StringName("generator_version"), 1));
	return v;
}

bool TemplateVersion::equals(const Ref<TemplateVersion> &p_other) const {
	return p_other.is_valid() && schema_version == p_other->schema_version && generator_version == p_other->generator_version;
}

int TemplateVersion::compare_to(const Ref<TemplateVersion> &p_other) const {
	if (p_other.is_null()) {
		return 1;
	}
	if (schema_version != p_other->schema_version) {
		return schema_version < p_other->schema_version ? -1 : 1;
	}
	if (generator_version != p_other->generator_version) {
		return generator_version < p_other->generator_version ? -1 : 1;
	}
	return 0;
}

bool TemplateVersion::is_compatible_with(const Ref<TemplateVersion> &p_other) const {
	return p_other.is_valid() && schema_version == p_other->schema_version;
}

String TemplateVersion::to_string() const {
	return itos(schema_version) + "." + itos(generator_version);
}

/**************************************************************************/
/* WorldCellCoord                                                         */
/**************************************************************************/

void WorldCellCoord::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_q", "q"), &WorldCellCoord::set_q);
	ClassDB::bind_method(D_METHOD("get_q"), &WorldCellCoord::get_q);
	ClassDB::bind_method(D_METHOD("set_r", "r"), &WorldCellCoord::set_r);
	ClassDB::bind_method(D_METHOD("get_r"), &WorldCellCoord::get_r);
	ClassDB::bind_method(D_METHOD("set_qr", "q", "r"), &WorldCellCoord::set_qr);
	ClassDB::bind_method(D_METHOD("is_valid"), &WorldCellCoord::is_valid);
	ClassDB::bind_method(D_METHOD("to_dict"), &WorldCellCoord::to_dict);
	ClassDB::bind_static_method("WorldCellCoord", D_METHOD("from_dict", "dict"), &WorldCellCoord::from_dict);
	ClassDB::bind_static_method("WorldCellCoord", D_METHOD("make", "q", "r"), &WorldCellCoord::make);
	ClassDB::bind_method(D_METHOD("equals", "other"), &WorldCellCoord::equals);
	ClassDB::bind_method(D_METHOD("compare_to", "other"), &WorldCellCoord::compare_to);
	ClassDB::bind_method(D_METHOD("hash"), &WorldCellCoord::hash);
	ClassDB::bind_method(D_METHOD("distance_to", "other"), &WorldCellCoord::distance_to);
	ClassDB::bind_method(D_METHOD("neighbors"), &WorldCellCoord::neighbors);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "q"), "set_q", "get_q");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "r"), "set_r", "get_r");
}

void WorldCellCoord::set_q(int32_t p_q) { q = p_q; }
int32_t WorldCellCoord::get_q() const { return q; }
void WorldCellCoord::set_r(int32_t p_r) { r = p_r; }
int32_t WorldCellCoord::get_r() const { return r; }
void WorldCellCoord::set_qr(int32_t p_q, int32_t p_r) {
	q = p_q;
	r = p_r;
}

Ref<WorldCellCoord> WorldCellCoord::make(int32_t p_q, int32_t p_r) {
	Ref<WorldCellCoord> c;
	c.instantiate();
	c->set_qr(p_q, p_r);
	return c;
}

bool WorldCellCoord::is_valid() const { return true; }

Dictionary WorldCellCoord::to_dict() const {
	Dictionary d;
	d[StringName("q")] = q;
	d[StringName("r")] = r;
	return d;
}

Ref<WorldCellCoord> WorldCellCoord::from_dict(const Dictionary &p_dict) {
	return make(p_dict.get(StringName("q"), 0), p_dict.get(StringName("r"), 0));
}

bool WorldCellCoord::equals(const Ref<WorldCellCoord> &p_other) const {
	return p_other.is_valid() && q == p_other->q && r == p_other->r;
}

int WorldCellCoord::compare_to(const Ref<WorldCellCoord> &p_other) const {
	if (p_other.is_null()) {
		return 1;
	}
	if (q != p_other->q) {
		return q < p_other->q ? -1 : 1;
	}
	if (r != p_other->r) {
		return r < p_other->r ? -1 : 1;
	}
	return 0;
}

uint32_t WorldCellCoord::hash() const {
	uint32_t h = hash_murmur3_one_32((uint32_t)q);
	h = hash_murmur3_one_32((uint32_t)r, h);
	return hash_fmix32(h);
}

static int32_t _cube_abs(int32_t v) { return v < 0 ? -v : v; }

int WorldCellCoord::distance_to(const Ref<WorldCellCoord> &p_other) const {
	if (p_other.is_null()) {
		return 0;
	}
	const int32_t dx = q - p_other->q;
	const int32_t dz = r - p_other->r;
	const int32_t dy = -dx - dz;
	return MAX(MAX(_cube_abs(dx), _cube_abs(dy)), _cube_abs(dz));
}

TypedArray<WorldCellCoord> WorldCellCoord::neighbors() const {
	static const int32_t DIRS[6][2] = {
		{ 1, 0 }, { 1, -1 }, { 0, -1 }, { -1, 0 }, { -1, 1 }, { 0, 1 }
	};
	TypedArray<WorldCellCoord> result;
	for (int i = 0; i < 6; i++) {
		result.append(make(q + DIRS[i][0], r + DIRS[i][1]));
	}
	return result;
}

String WorldCellCoord::to_string() const {
	return "(" + itos(q) + ", " + itos(r) + ")";
}

/**************************************************************************/
/* MicroCoord                                                             */
/**************************************************************************/

void MicroCoord::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_q", "q"), &MicroCoord::set_q);
	ClassDB::bind_method(D_METHOD("get_q"), &MicroCoord::get_q);
	ClassDB::bind_method(D_METHOD("set_r", "r"), &MicroCoord::set_r);
	ClassDB::bind_method(D_METHOD("get_r"), &MicroCoord::get_r);
	ClassDB::bind_method(D_METHOD("set_qr", "q", "r"), &MicroCoord::set_qr);
	ClassDB::bind_method(D_METHOD("set_detail_level", "level"), &MicroCoord::set_detail_level);
	ClassDB::bind_method(D_METHOD("get_detail_level"), &MicroCoord::get_detail_level);
	ClassDB::bind_method(D_METHOD("is_valid"), &MicroCoord::is_valid);
	ClassDB::bind_method(D_METHOD("to_dict"), &MicroCoord::to_dict);
	ClassDB::bind_static_method("MicroCoord", D_METHOD("from_dict", "dict"), &MicroCoord::from_dict);
	ClassDB::bind_static_method("MicroCoord", D_METHOD("make", "q", "r", "detail_level"), &MicroCoord::make, DEFVAL(1));
	ClassDB::bind_method(D_METHOD("equals", "other"), &MicroCoord::equals);
	ClassDB::bind_method(D_METHOD("compare_to", "other"), &MicroCoord::compare_to);
	ClassDB::bind_method(D_METHOD("hash"), &MicroCoord::hash);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "q"), "set_q", "get_q");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "r"), "set_r", "get_r");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "detail_level"), "set_detail_level", "get_detail_level");
}

void MicroCoord::set_q(int32_t p_q) { q = p_q; }
int32_t MicroCoord::get_q() const { return q; }
void MicroCoord::set_r(int32_t p_r) { r = p_r; }
int32_t MicroCoord::get_r() const { return r; }
void MicroCoord::set_qr(int32_t p_q, int32_t p_r) {
	q = p_q;
	r = p_r;
}

Ref<MicroCoord> MicroCoord::make(int32_t p_q, int32_t p_r, int p_detail_level) {
	Ref<MicroCoord> c;
	c.instantiate();
	c->set_qr(p_q, p_r);
	c->set_detail_level(p_detail_level);
	return c;
}

void MicroCoord::set_detail_level(int p_level) { detail_level = p_level; }
int MicroCoord::get_detail_level() const { return detail_level; }
bool MicroCoord::is_valid() const { return detail_level >= 1; }

Dictionary MicroCoord::to_dict() const {
	Dictionary d;
	d[StringName("q")] = q;
	d[StringName("r")] = r;
	d[StringName("detail_level")] = detail_level;
	return d;
}

Ref<MicroCoord> MicroCoord::from_dict(const Dictionary &p_dict) {
	return make(p_dict.get(StringName("q"), 0), p_dict.get(StringName("r"), 0), p_dict.get(StringName("detail_level"), 1));
}

bool MicroCoord::equals(const Ref<MicroCoord> &p_other) const {
	return p_other.is_valid() && q == p_other->q && r == p_other->r && detail_level == p_other->detail_level;
}

int MicroCoord::compare_to(const Ref<MicroCoord> &p_other) const {
	if (p_other.is_null()) {
		return 1;
	}
	if (q != p_other->q) {
		return q < p_other->q ? -1 : 1;
	}
	if (r != p_other->r) {
		return r < p_other->r ? -1 : 1;
	}
	if (detail_level != p_other->detail_level) {
		return detail_level < p_other->detail_level ? -1 : 1;
	}
	return 0;
}

uint32_t MicroCoord::hash() const {
	uint32_t h = hash_murmur3_one_32((uint32_t)q);
	h = hash_murmur3_one_32((uint32_t)r, h);
	h = hash_murmur3_one_32((uint32_t)detail_level, h);
	return hash_fmix32(h);
}

String MicroCoord::to_string() const {
	return "(" + itos(q) + ", " + itos(r) + "; L" + itos(detail_level) + ")";
}

/**************************************************************************/
/* FeatureId                                                              */
/**************************************************************************/

void FeatureId::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_kind", "kind"), &FeatureId::set_kind);
	ClassDB::bind_method(D_METHOD("get_kind"), &FeatureId::get_kind);
	ClassDB::bind_method(D_METHOD("set_seq", "seq"), &FeatureId::set_seq);
	ClassDB::bind_method(D_METHOD("get_seq"), &FeatureId::get_seq);
	ClassDB::bind_method(D_METHOD("is_valid"), &FeatureId::is_valid);
	ClassDB::bind_method(D_METHOD("to_dict"), &FeatureId::to_dict);
	ClassDB::bind_static_method("FeatureId", D_METHOD("from_dict", "dict"), &FeatureId::from_dict);
	ClassDB::bind_static_method("FeatureId", D_METHOD("make", "kind", "seq"), &FeatureId::make);
	ClassDB::bind_method(D_METHOD("equals", "other"), &FeatureId::equals);
	ClassDB::bind_method(D_METHOD("compare_to", "other"), &FeatureId::compare_to);
	ClassDB::bind_method(D_METHOD("hash"), &FeatureId::hash);
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "kind"), "set_kind", "get_kind");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "seq"), "set_seq", "get_seq");
}

void FeatureId::set_kind(const StringName &p_kind) { kind = p_kind; }
StringName FeatureId::get_kind() const { return kind; }
void FeatureId::set_seq(int64_t p_seq) { seq = p_seq; }
int64_t FeatureId::get_seq() const { return seq; }
bool FeatureId::is_valid() const { return kind != StringName() && seq > 0; }

Dictionary FeatureId::to_dict() const {
	Dictionary d;
	d[StringName("kind")] = kind;
	d[StringName("seq")] = seq;
	return d;
}

Ref<FeatureId> FeatureId::from_dict(const Dictionary &p_dict) {
	return make(p_dict.get(StringName("kind"), StringName()), p_dict.get(StringName("seq"), (int64_t)0));
}

Ref<FeatureId> FeatureId::make(const StringName &p_kind, int64_t p_seq) {
	Ref<FeatureId> id;
	id.instantiate();
	id->set_kind(p_kind);
	id->set_seq(p_seq);
	return id;
}

bool FeatureId::equals(const Ref<FeatureId> &p_other) const {
	return p_other.is_valid() && kind == p_other->kind && seq == p_other->seq;
}

int FeatureId::compare_to(const Ref<FeatureId> &p_other) const {
	if (p_other.is_null()) {
		return 1;
	}
	int kind_cmp = String(kind).casecmp_to(String(p_other->kind));
	if (kind_cmp != 0) {
		return kind_cmp < 0 ? -1 : 1;
	}
	if (seq != p_other->seq) {
		return seq < p_other->seq ? -1 : 1;
	}
	return 0;
}

uint32_t FeatureId::hash() const {
	uint32_t h = kind.hash();
	h = hash_murmur3_one_64((uint64_t)seq, h);
	return hash_fmix32(h);
}

String FeatureId::to_string() const {
	return String(kind) + "#" + String::num_int64(seq);
}

/**************************************************************************/
/* EdgeKey                                                                */
/**************************************************************************/

void EdgeKey::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_endpoint_a", "coord"), &EdgeKey::set_endpoint_a);
	ClassDB::bind_method(D_METHOD("get_endpoint_a"), &EdgeKey::get_endpoint_a);
	ClassDB::bind_method(D_METHOD("set_endpoint_b", "coord"), &EdgeKey::set_endpoint_b);
	ClassDB::bind_method(D_METHOD("get_endpoint_b"), &EdgeKey::get_endpoint_b);
	ClassDB::bind_method(D_METHOD("is_valid"), &EdgeKey::is_valid);
	ClassDB::bind_method(D_METHOD("to_dict"), &EdgeKey::to_dict);
	ClassDB::bind_static_method("EdgeKey", D_METHOD("from_dict", "dict"), &EdgeKey::from_dict);
	ClassDB::bind_static_method("EdgeKey", D_METHOD("make", "a", "b"), &EdgeKey::make);
	ClassDB::bind_method(D_METHOD("equals", "other"), &EdgeKey::equals);
	ClassDB::bind_method(D_METHOD("compare_to", "other"), &EdgeKey::compare_to);
	ClassDB::bind_method(D_METHOD("hash"), &EdgeKey::hash);
}

void EdgeKey::set_endpoint_a(const Ref<WorldCellCoord> &p_coord) { endpoint_a = p_coord; }
Ref<WorldCellCoord> EdgeKey::get_endpoint_a() const { return endpoint_a; }
void EdgeKey::set_endpoint_b(const Ref<WorldCellCoord> &p_coord) { endpoint_b = p_coord; }
Ref<WorldCellCoord> EdgeKey::get_endpoint_b() const { return endpoint_b; }

bool EdgeKey::is_valid() const {
	return endpoint_a.is_valid() && endpoint_b.is_valid() && !endpoint_a->equals(endpoint_b);
}

Ref<EdgeKey> EdgeKey::make(const Ref<WorldCellCoord> &p_a, const Ref<WorldCellCoord> &p_b) {
	Ref<EdgeKey> key;
	key.instantiate();
	if (p_a.is_valid() && p_b.is_valid() && p_a->compare_to(p_b) <= 0) {
		key->set_endpoint_a(p_a);
		key->set_endpoint_b(p_b);
	} else {
		key->set_endpoint_a(p_b);
		key->set_endpoint_b(p_a);
	}
	return key;
}

Dictionary EdgeKey::to_dict() const {
	Dictionary d;
	Dictionary da = endpoint_a.is_valid() ? endpoint_a->to_dict() : Dictionary();
	Dictionary db = endpoint_b.is_valid() ? endpoint_b->to_dict() : Dictionary();
	d[StringName("a")] = da;
	d[StringName("b")] = db;
	return d;
}

Ref<EdgeKey> EdgeKey::from_dict(const Dictionary &p_dict) {
	Dictionary da = p_dict.get(StringName("a"), Dictionary());
	Dictionary db = p_dict.get(StringName("b"), Dictionary());
	return make(WorldCellCoord::from_dict(da), WorldCellCoord::from_dict(db));
}

bool EdgeKey::equals(const Ref<EdgeKey> &p_other) const {
	if (p_other.is_null()) {
		return false;
	}
	bool ab = endpoint_a.is_valid() && p_other->endpoint_a.is_valid() && endpoint_a->equals(p_other->endpoint_a);
	bool bb = endpoint_b.is_valid() && p_other->endpoint_b.is_valid() && endpoint_b->equals(p_other->endpoint_b);
	return ab && bb;
}

int EdgeKey::compare_to(const Ref<EdgeKey> &p_other) const {
	if (p_other.is_null()) {
		return 1;
	}
	int cmp = endpoint_a.is_valid() && p_other->endpoint_a.is_valid() ? endpoint_a->compare_to(p_other->endpoint_a) : 0;
	if (cmp != 0) {
		return cmp;
	}
	if (endpoint_b.is_valid() && p_other->endpoint_b.is_valid()) {
		return endpoint_b->compare_to(p_other->endpoint_b);
	}
	return 0;
}

uint32_t EdgeKey::hash() const {
	uint32_t h = endpoint_a.is_valid() ? endpoint_a->hash() : 0;
	h = hash_murmur3_one_32(endpoint_b.is_valid() ? endpoint_b->hash() : 0, h);
	return hash_fmix32(h);
}

String EdgeKey::to_string() const {
	String sa = endpoint_a.is_valid() ? endpoint_a->to_string() : "<null>";
	String sb = endpoint_b.is_valid() ? endpoint_b->to_string() : "<null>";
	return sa + " -- " + sb;
}
