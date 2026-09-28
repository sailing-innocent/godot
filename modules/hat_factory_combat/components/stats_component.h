#ifndef STATS_COMPONENT_H
#define STATS_COMPONENT_H

#include "combat_component.h"

class StatsComponent : public CombatComponent {
	GDCLASS(StatsComponent, CombatComponent)

	int hp = 0;
	int max_hp = 0;
	int mp = 0;
	int max_mp = 0;
	int ap = 0;
	int max_ap = 0;

protected:
	static void _bind_methods();

public:
	virtual StringName get_component_name() const override { return StringName("Stats"); }

	void set_hp(int p_value) { hp = p_value; }
	void set_max_hp(int p_value) { max_hp = p_value; }
	void set_mp(int p_value) { mp = p_value; }
	void set_max_mp(int p_value) { max_mp = p_value; }
	void set_ap(int p_value) { ap = p_value; }
	void set_max_ap(int p_value) { max_ap = p_value; }
	int get_hp() const { return hp; }
	int get_max_hp() const { return max_hp; }
	int get_mp() const { return mp; }
	int get_max_mp() const { return max_mp; }
	int get_ap() const { return ap; }
	int get_max_ap() const { return max_ap; }

	virtual Dictionary to_dict() const override;
	virtual void from_dict(const Dictionary &p_dict) override;
	virtual Ref<CombatComponent> clone() const override;
};

#endif // STATS_COMPONENT_H
