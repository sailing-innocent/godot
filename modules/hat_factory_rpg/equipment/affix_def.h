#ifndef HAT_FACTORY_RPG_AFFIX_DEF_H
#define HAT_FACTORY_RPG_AFFIX_DEF_H

#include "core/io/resource.h"
#include "core/string/string_name.h"
#include "core/string/ustring.h"
#include "core/variant/dictionary.h"

class AffixDef : public Resource {
	GDCLASS(AffixDef, Resource)

	StringName affix_id;
	String display_name;
	StringName affected_attr;
	int min_value = 0;
	int max_value = 0;
	int weight = 100;

protected:
	static void _bind_methods();

public:
	void set_affix_id(const StringName &p_id) { affix_id = p_id; }
	StringName get_affix_id() const { return affix_id; }

	void set_display_name(const String &p_name) { display_name = p_name; }
	String get_display_name() const { return display_name; }

	void set_affected_attr(const StringName &p_attr) { affected_attr = p_attr; }
	StringName get_affected_attr() const { return affected_attr; }

	void set_min_value(int p_value) { min_value = p_value; }
	int get_min_value() const { return min_value; }

	void set_max_value(int p_value) { max_value = p_value; }
	int get_max_value() const { return max_value; }

	void set_weight(int p_weight) { weight = p_weight; }
	int get_weight() const { return weight; }

	Dictionary to_dict() const;
	void from_dict(const Dictionary &p_dict);
	Ref<AffixDef> clone() const;
};

#endif // HAT_FACTORY_RPG_AFFIX_DEF_H
