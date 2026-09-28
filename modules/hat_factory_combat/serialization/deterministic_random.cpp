#include "deterministic_random.h"

#include "core/object/class_db.h"

void DeterministicRandom::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_seed", "seed"), &DeterministicRandom::set_seed);
	ClassDB::bind_method(D_METHOD("get_seed"), &DeterministicRandom::get_seed);
	ClassDB::bind_method(D_METHOD("set_index", "index"), &DeterministicRandom::set_index);
	ClassDB::bind_method(D_METHOD("get_index"), &DeterministicRandom::get_index);
	ClassDB::bind_method(D_METHOD("next_int", "min", "max"), &DeterministicRandom::next_int);
	ClassDB::bind_method(D_METHOD("next_float"), &DeterministicRandom::next_float);
	ClassDB::bind_method(D_METHOD("to_dict"), &DeterministicRandom::to_dict);
	ClassDB::bind_method(D_METHOD("clone"), &DeterministicRandom::clone);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "seed"), "set_seed", "get_seed");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "index"), "set_index", "get_index");
}

uint32_t DeterministicRandom::_hash32(uint32_t p_value) {
	// Simple xorshift-based hash
	p_value ^= p_value << 13;
	p_value ^= p_value >> 17;
	p_value ^= p_value << 5;
	return p_value;
}

void DeterministicRandom::set_seed(uint32_t p_seed) {
	seed = p_seed == 0 ? 12345 : p_seed;
	index = 0;
}

int DeterministicRandom::next_int(int p_min, int p_max) {
	if (p_min >= p_max) return p_min;
	uint32_t mix = seed ^ _hash32((uint32_t)index);
	index++;
	uint32_t range = (uint32_t)(p_max - p_min);
	return p_min + (int)(mix % range);
}

float DeterministicRandom::next_float() {
	uint32_t mix = seed ^ _hash32((uint32_t)index);
	index++;
	return (float)(mix & 0x7fffff) / (float)0x7fffff;
}

Dictionary DeterministicRandom::to_dict() const {
	Dictionary d;
	d["seed"] = (int)seed;
	d["index"] = (int64_t)index;
	return d;
}

Ref<DeterministicRandom> DeterministicRandom::from_dict(const Dictionary &p_dict) {
	Ref<DeterministicRandom> r;
	r.instantiate();
	if (p_dict.has("seed")) r->seed = (uint32_t)(int)p_dict["seed"];
	if (p_dict.has("index")) r->index = (uint64_t)(int64_t)p_dict["index"];
	return r;
}

Ref<DeterministicRandom> DeterministicRandom::clone() const {
	Ref<DeterministicRandom> r;
	r.instantiate();
	r->seed = seed;
	r->index = index;
	return r;
}
