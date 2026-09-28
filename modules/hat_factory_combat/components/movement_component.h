#ifndef MOVEMENT_COMPONENT_H
#define MOVEMENT_COMPONENT_H

#include "combat_component.h"

class MovementComponent : public CombatComponent {
	GDCLASS(MovementComponent, CombatComponent)

	int range = 0;
	bool flying = false;
	int jump_height = 0;

protected:
	static void _bind_methods();

public:
	virtual StringName get_component_name() const override { return StringName("Movement"); }

	void set_range(int p_value) { range = p_value; }
	void set_flying(bool p_value) { flying = p_value; }
	void set_jump_height(int p_value) { jump_height = p_value; }
	int get_range() const { return range; }
	bool get_flying() const { return flying; }
	int get_jump_height() const { return jump_height; }

	virtual Dictionary to_dict() const override;
	virtual void from_dict(const Dictionary &p_dict) override;
	virtual Ref<CombatComponent> clone() const override;
};

#endif // MOVEMENT_COMPONENT_H
