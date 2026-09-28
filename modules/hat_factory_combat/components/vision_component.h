#ifndef VISION_COMPONENT_H
#define VISION_COMPONENT_H

#include "combat_component.h"

class VisionComponent : public CombatComponent {
	GDCLASS(VisionComponent, CombatComponent)

	int range = 0;
	int height_bonus = 0;

protected:
	static void _bind_methods();

public:
	virtual StringName get_component_name() const override { return StringName("Vision"); }

	void set_range(int p_value) { range = p_value; }
	void set_height_bonus(int p_value) { height_bonus = p_value; }
	int get_range() const { return range; }
	int get_height_bonus() const { return height_bonus; }

	virtual Dictionary to_dict() const override;
	virtual void from_dict(const Dictionary &p_dict) override;
	virtual Ref<CombatComponent> clone() const override;
};

#endif // VISION_COMPONENT_H
