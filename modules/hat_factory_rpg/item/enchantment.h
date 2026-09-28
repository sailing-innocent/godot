#ifndef HAT_FACTORY_RPG_ENCHANTMENT_H
#define HAT_FACTORY_RPG_ENCHANTMENT_H

#include "core/object/ref_counted.h"
#include "core/string/string_name.h"
#include "core/variant/dictionary.h"

class Enchantment : public RefCounted {
	GDCLASS(Enchantment, RefCounted)

	StringName affix_id;
	int value = 0;
	StringName affected_attr;

protected:
	static void _bind_methods();

public:
	void set_affix_id(const StringName &p_id) { affix_id = p_id; }
	StringName get_affix_id() const { return affix_id; }

	void set_value(int p_value) { value = p_value; }
	int get_value() const { return value; }

	void set_affected_attr(const StringName &p_attr) { affected_attr = p_attr; }
	StringName get_affected_attr() const { return affected_attr; }

	Dictionary to_dict() const;
	static Ref<Enchantment> from_dict(const Dictionary &p_dict);
	Ref<Enchantment> clone() const;
};

#endif // HAT_FACTORY_RPG_ENCHANTMENT_H
