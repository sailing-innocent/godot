#include "turn_component.h"

#include "core/object/class_db.h"

void TurnComponent::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_moved", "moved"), &TurnComponent::set_moved);
	ClassDB::bind_method(D_METHOD("set_acted", "acted"), &TurnComponent::set_acted);
	ClassDB::bind_method(D_METHOD("set_skipped", "skipped"), &TurnComponent::set_skipped);
	ClassDB::bind_method(D_METHOD("get_moved"), &TurnComponent::get_moved);
	ClassDB::bind_method(D_METHOD("get_acted"), &TurnComponent::get_acted);
	ClassDB::bind_method(D_METHOD("get_skipped"), &TurnComponent::get_skipped);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "moved"), "set_moved", "get_moved");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "acted"), "set_acted", "get_acted");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "skipped"), "set_skipped", "get_skipped");
}

Dictionary TurnComponent::to_dict() const {
	Dictionary d;
	d["moved"] = moved;
	d["acted"] = acted;
	d["skipped"] = skipped;
	return d;
}

void TurnComponent::from_dict(const Dictionary &p_dict) {
	if (p_dict.has("moved")) { moved = p_dict["moved"]; }
	if (p_dict.has("acted")) { acted = p_dict["acted"]; }
	if (p_dict.has("skipped")) { skipped = p_dict["skipped"]; }
}

Ref<CombatComponent> TurnComponent::clone() const {
	Ref<TurnComponent> copy;
	copy.instantiate();
	copy->from_dict(to_dict());
	return copy;
}
