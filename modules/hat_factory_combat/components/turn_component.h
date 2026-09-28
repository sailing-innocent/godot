#ifndef TURN_COMPONENT_H
#define TURN_COMPONENT_H

#include "combat_component.h"

class TurnComponent : public CombatComponent {
	GDCLASS(TurnComponent, CombatComponent)

	bool moved = false;
	bool acted = false;
	bool skipped = false;

protected:
	static void _bind_methods();

public:
	virtual StringName get_component_name() const override { return StringName("Turn"); }

	void set_moved(bool p_value) { moved = p_value; }
	void set_acted(bool p_value) { acted = p_value; }
	void set_skipped(bool p_value) { skipped = p_value; }
	bool get_moved() const { return moved; }
	bool get_acted() const { return acted; }
	bool get_skipped() const { return skipped; }

	virtual Dictionary to_dict() const override;
	virtual void from_dict(const Dictionary &p_dict) override;
	virtual Ref<CombatComponent> clone() const override;
};

#endif // TURN_COMPONENT_H
