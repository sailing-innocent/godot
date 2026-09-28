#ifndef IDENTITY_COMPONENT_H
#define IDENTITY_COMPONENT_H

#include "combat_component.h"

class IdentityComponent : public CombatComponent {
	GDCLASS(IdentityComponent, CombatComponent)

	StringName def_id;
	String display_name;
	String sub_name;

protected:
	static void _bind_methods();

public:
	virtual StringName get_component_name() const override { return StringName("Identity"); }

	void set_def_id(StringName p_value) { def_id = p_value; }
	void set_display_name(String p_value) { display_name = p_value; }
	void set_sub_name(String p_value) { sub_name = p_value; }
	StringName get_def_id() const { return def_id; }
	String get_display_name() const { return display_name; }
	String get_sub_name() const { return sub_name; }

	virtual Dictionary to_dict() const override;
	virtual void from_dict(const Dictionary &p_dict) override;
	virtual Ref<CombatComponent> clone() const override;
};

#endif // IDENTITY_COMPONENT_H
