#ifndef HEX_CELL_EFFECT_H
#define HEX_CELL_EFFECT_H

#include "core/io/resource.h"

/**
 * @brief Base class for per-cell gameplay effects.
 *
 * Mods and the creative workshop can extend this Resource in GDScript
 * and attach instances to HexCellData. The engine module only stores and
 * serializes the effects; the game logic interprets them.
 */
class HexCellEffect : public Resource {
	GDCLASS(HexCellEffect, Resource)

protected:
	static void _bind_methods() {}
};

#endif // HEX_CELL_EFFECT_H
