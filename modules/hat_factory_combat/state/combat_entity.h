#ifndef COMBAT_ENTITY_H
#define COMBAT_ENTITY_H

#include "core/object/ref_counted.h"
#include "core/string/string_name.h"
#include "core/variant/dictionary.h"
#include "core/variant/typed_array.h"
#include "core/templates/hash_map.h"

class CombatComponent;

class CombatEntity : public RefCounted {
	GDCLASS(CombatEntity, RefCounted)

	int entity_id = 0;
	int owner = 0;
	StringName type;

	HashMap<StringName, Ref<CombatComponent>> components;

protected:
	static void _bind_methods();

public:
	void set_entity_id(int p_id) { entity_id = p_id; }
	int get_entity_id() const { return entity_id; }

	void set_owner(int p_owner) { owner = p_owner; }
	int get_owner() const { return owner; }

	void set_type(const StringName &p_type) { type = p_type; }
	StringName get_type() const { return type; }

	void add_component(const Ref<CombatComponent> &p_component);
	void remove_component(const StringName &p_name);
	bool has_component(const StringName &p_name) const;
	Ref<CombatComponent> get_component(const StringName &p_name) const;
	TypedArray<StringName> get_component_names() const;

	Dictionary to_dict() const;
	static Ref<CombatEntity> from_dict(const Dictionary &p_dict);
	Ref<CombatEntity> clone() const;
};

#endif // COMBAT_ENTITY_H
