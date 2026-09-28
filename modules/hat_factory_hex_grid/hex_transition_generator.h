#ifndef HEX_TRANSITION_GENERATOR_H
#define HEX_TRANSITION_GENERATOR_H

#include "hex_transition_instance_data.h"
#include "hex_transition_library.h"
#include "core/object/ref_counted.h"
#include "core/variant/typed_array.h"

class HexGridMap;

/**
 * @brief Generates transition instance data for a HexGridMap.
 */
class HexTransitionGenerator : public RefCounted {
	GDCLASS(HexTransitionGenerator, RefCounted)

	HexGridMap *map = nullptr;
	Ref<HexTransitionLibrary> library;

protected:
	static void _bind_methods();

public:
	void setup(HexGridMap *p_map, const Ref<HexTransitionLibrary> &p_library);

	TypedArray<HexTransitionInstanceData> generate_for_coord(const Vector2i &p_coord) const;
	TypedArray<HexTransitionInstanceData> generate_all() const;

	HexTransitionGenerator() = default;
};

#endif // HEX_TRANSITION_GENERATOR_H
