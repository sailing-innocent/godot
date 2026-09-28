#ifndef DETERMINISTIC_RANDOM_H
#define DETERMINISTIC_RANDOM_H

#include "core/object/ref_counted.h"

class DeterministicRandom : public RefCounted {
	GDCLASS(DeterministicRandom, RefCounted)

	uint32_t seed = 0;
	uint64_t index = 0;

protected:
	static void _bind_methods();

	static uint32_t _hash32(uint32_t p_value);

public:
	void set_seed(uint32_t p_seed);
	uint32_t get_seed() const { return seed; }

	void set_index(uint64_t p_index) { index = p_index; }
	uint64_t get_index() const { return index; }

	int next_int(int p_min, int p_max);
	float next_float();

	Dictionary to_dict() const;
	static Ref<DeterministicRandom> from_dict(const Dictionary &p_dict);
	Ref<DeterministicRandom> clone() const;
};

#endif // DETERMINISTIC_RANDOM_H
