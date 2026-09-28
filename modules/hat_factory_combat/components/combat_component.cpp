#include "combat_component.h"

#include "core/object/class_db.h"

void CombatComponent::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_component_name"), &CombatComponent::get_component_name);
	ClassDB::bind_method(D_METHOD("to_dict"), &CombatComponent::to_dict);
	ClassDB::bind_method(D_METHOD("from_dict", "dict"), &CombatComponent::from_dict);
	ClassDB::bind_method(D_METHOD("clone"), &CombatComponent::clone);
}

StringName CombatComponent::get_component_name() const {
	return StringName();
}

Dictionary CombatComponent::to_dict() const {
	return Dictionary();
}

void CombatComponent::from_dict(const Dictionary &p_dict) {
}

Ref<CombatComponent> CombatComponent::clone() const {
	Ref<CombatComponent> copy;
	copy.instantiate();
	copy->from_dict(to_dict());
	return copy;
}
