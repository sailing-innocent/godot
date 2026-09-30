/**************************************************************************/
/*  hex_tag_modifier_table.h                                              */
/*  Hat Factory World module — tag -> numeric stat mapping (MAP-26).      */
/*                                                                        */
/*  Migration target of RuleSet.conceal_terrain_id / energy_recovery_*:   */
/*  combat reads stats through this table instead of hardcoded terrain    */
/*  ids. Legacy RuleSet fields stay during a deprecation window.          */
/**************************************************************************/
#ifndef HEX_TAG_MODIFIER_TABLE_H
#define HEX_TAG_MODIFIER_TABLE_H

#include "core/io/resource.h"
#include "core/string/string_name.h"
#include "core/variant/dictionary.h"

class HexTagModifierTable : public Resource {
	GDCLASS(HexTagModifierTable, Resource)
	StringName table_id;
	// tag -> { "stat_name": value, ... }  (e.g. "concealed" -> {"concealment": 1.0})
	Dictionary modifiers;

protected:
	static void _bind_methods();

public:
	void set_table_id(const StringName &p_id);
	StringName get_table_id() const;
	void set_modifiers(const Dictionary &p_modifiers);
	Dictionary get_modifiers() const;

	void set_value(const StringName &p_tag, const StringName &p_stat, double p_value);
	/** Value for (tag, stat); p_default when the tag or stat is absent. */
	double get_value(const StringName &p_tag, const StringName &p_stat, double p_default = 0.0) const;
	Dictionary get_stats_for_tag(const StringName &p_tag) const;
	Dictionary to_dict() const;
	static Ref<HexTagModifierTable> from_dict(const Dictionary &p_dict);
};

#endif // HEX_TAG_MODIFIER_TABLE_H
