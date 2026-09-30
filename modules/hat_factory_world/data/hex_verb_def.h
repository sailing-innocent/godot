/**************************************************************************/
/*  hex_verb_def.h                                                        */
/*  Hat Factory World module — interaction verbs (MAP-30/31).            */
/*                                                                        */
/*  Each verb declares capability requirements, cost and the map change  */
/*  it produces (a MapCommand template resolved by the command resolver). */
/**************************************************************************/
#ifndef HEX_VERB_DEF_H
#define HEX_VERB_DEF_H

#include "core/io/resource.h"
#include "core/string/string_name.h"
#include "core/variant/dictionary.h"

class HexVerbDef : public Resource {
	GDCLASS(HexVerbDef, Resource)
	StringName verb_id;
	String display_name;
	StringName target_kind = StringName("cell"); // cell | edge | object
	PackedStringArray required_hat_tags;
	StringName required_item_id; // empty = none
	StringName required_key_id; // empty = none (MAP-33)
	int ap_cost = 0;
	int time_ms = 0; // QTE windows reference profiles by id elsewhere; this is base time
	Dictionary command_template; // MapCommand payload template; resolver fills coords/versions

protected:
	static void _bind_methods();

public:
	void set_verb_id(const StringName &p_id);
	StringName get_verb_id() const;
	/** Uniform id accessor shared with layer defs (table compilation). */
	StringName get_def_id() const { return get_verb_id(); }
	void set_display_name(const String &p_name);
	String get_display_name() const;
	void set_target_kind(const StringName &p_kind);
	StringName get_target_kind() const;
	void set_required_hat_tags(const PackedStringArray &p_tags);
	PackedStringArray get_required_hat_tags() const;
	void set_required_item_id(const StringName &p_item);
	StringName get_required_item_id() const;
	void set_required_key_id(const StringName &p_key);
	StringName get_required_key_id() const;
	void set_ap_cost(int p_cost);
	int get_ap_cost() const;
	void set_time_ms(const int p_ms);
	int get_time_ms() const;
	void set_command_template(const Dictionary &p_template);
	Dictionary get_command_template() const;
	Dictionary to_dict() const;
	static Ref<HexVerbDef> from_dict(const Dictionary &p_dict);
	String validate() const;
};

#endif // HEX_VERB_DEF_H
