#include "combat_entity.h"

#include "components/combat_component.h"
#include "core/object/class_db.h"
#include "core/variant/typed_array.h"

void CombatEntity::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_entity_id", "entity_id"), &CombatEntity::set_entity_id);
	ClassDB::bind_method(D_METHOD("get_entity_id"), &CombatEntity::get_entity_id);
	ClassDB::bind_method(D_METHOD("set_owner", "owner"), &CombatEntity::set_owner);
	ClassDB::bind_method(D_METHOD("get_owner"), &CombatEntity::get_owner);
	ClassDB::bind_method(D_METHOD("set_type", "type"), &CombatEntity::set_type);
	ClassDB::bind_method(D_METHOD("get_type"), &CombatEntity::get_type);
	ClassDB::bind_method(D_METHOD("add_component", "component"), &CombatEntity::add_component);
	ClassDB::bind_method(D_METHOD("remove_component", "name"), &CombatEntity::remove_component);
	ClassDB::bind_method(D_METHOD("has_component", "name"), &CombatEntity::has_component);
	ClassDB::bind_method(D_METHOD("get_component", "name"), &CombatEntity::get_component);
	ClassDB::bind_method(D_METHOD("get_component_names"), &CombatEntity::get_component_names);
	ClassDB::bind_method(D_METHOD("to_dict"), &CombatEntity::to_dict);
	ClassDB::bind_static_method("CombatEntity", D_METHOD("from_dict", "dict"), &CombatEntity::from_dict);
	ClassDB::bind_method(D_METHOD("clone"), &CombatEntity::clone);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "entity_id"), "set_entity_id", "get_entity_id");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "owner"), "set_owner", "get_owner");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "type"), "set_type", "get_type");
}

void CombatEntity::add_component(const Ref<CombatComponent> &p_component) {
	if (p_component.is_null()) return;
	StringName name = p_component->get_component_name();
	if (name != StringName()) {
		components[name] = p_component;
	}
}

void CombatEntity::remove_component(const StringName &p_name) {
	components.erase(p_name);
}

bool CombatEntity::has_component(const StringName &p_name) const {
	return components.has(p_name);
}

Ref<CombatComponent> CombatEntity::get_component(const StringName &p_name) const {
	const Ref<CombatComponent> *comp = components.getptr(p_name);
	return comp != nullptr ? *comp : Ref<CombatComponent>();
}

TypedArray<StringName> CombatEntity::get_component_names() const {
	TypedArray<StringName> names;
	for (KeyValue<StringName, Ref<CombatComponent>> kv : components) {
		names.push_back(kv.key);
	}
	return names;
}

Dictionary CombatEntity::to_dict() const {
	Dictionary d;
	d["entity_id"] = entity_id;
	d["owner"] = owner;
	d["type"] = type;
	Dictionary comps;
	for (KeyValue<StringName, Ref<CombatComponent>> kv : components) {
		if (kv.value.is_valid()) {
			comps[kv.key] = kv.value->to_dict();
		}
	}
	d["components"] = comps;
	return d;
}

static Ref<CombatComponent> _instantiate_component_from_dict(const StringName &p_name, const Dictionary &p_data) {
	String class_name = String(p_name) + "Component";
	Object *obj = ClassDB::instantiate(class_name);
	if (!obj) {
		obj = ClassDB::instantiate(p_name);
	}
	if (!obj) {
		return Ref<CombatComponent>();
	}
	CombatComponent *comp = Object::cast_to<CombatComponent>(obj);
	if (!comp) {
		memdelete(obj);
		return Ref<CombatComponent>();
	}
	Ref<CombatComponent> ref;
	ref = comp;
	ref->from_dict(p_data);
	return ref;
}

Ref<CombatEntity> CombatEntity::from_dict(const Dictionary &p_dict) {
	Ref<CombatEntity> e;
	e.instantiate();
	if (p_dict.has("entity_id")) e->entity_id = p_dict["entity_id"];
	if (p_dict.has("owner")) e->owner = p_dict["owner"];
	if (p_dict.has("type")) e->type = p_dict["type"];
	if (p_dict.has("components")) {
		Dictionary comps = p_dict["components"];
		Array keys = comps.keys();
		for (int i = 0; i < keys.size(); i++) {
			StringName name = keys[i];
			Dictionary data = comps[name];
			Ref<CombatComponent> comp = _instantiate_component_from_dict(name, data);
			if (comp.is_valid()) {
				e->add_component(comp);
			}
		}
	}
	return e;
}

Ref<CombatEntity> CombatEntity::clone() const {
	Ref<CombatEntity> e;
	e.instantiate();
	e->entity_id = entity_id;
	e->owner = owner;
	e->type = type;
	for (KeyValue<StringName, Ref<CombatComponent>> kv : components) {
		if (kv.value.is_valid()) {
			e->components[kv.key] = kv.value->clone();
		}
	}
	return e;
}
