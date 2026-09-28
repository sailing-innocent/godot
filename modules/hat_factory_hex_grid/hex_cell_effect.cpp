#include "hex_cell_effect.h"

#include "core/object/class_db.h"

void HexCellEffect::_bind_methods() {
	ClassDB::bind_method(D_METHOD("to_dict"), &HexCellEffect::to_dict);
	ClassDB::bind_method(D_METHOD("clone"), &HexCellEffect::clone);
}

Dictionary HexCellEffect::to_dict() const {
	Dictionary d;
	if (get_script_instance()) {
		d["script"] = get_script();
	}
	return d;
}

Ref<HexCellEffect> HexCellEffect::clone() const {
	Ref<HexCellEffect> copy = duplicate();
	return copy;
}
