#ifndef COMBAT_ATTR_COMPONENT_H
#define COMBAT_ATTR_COMPONENT_H

#include "combat_component.h"

class CombatAttrComponent : public CombatComponent {
	GDCLASS(CombatAttrComponent, CombatComponent)

	int attack = 0;
	int defense = 0;
	int magic = 0;
	int resistance = 0;
	int aim = 0;
	int evasion = 0;
	float crit_rate = 0.0f;
	float crit_damage = 0.0f;

protected:
	static void _bind_methods();

public:
	virtual StringName get_component_name() const override { return StringName("CombatAttr"); }

	void set_attack(int p_value) { attack = p_value; }
	void set_defense(int p_value) { defense = p_value; }
	void set_magic(int p_value) { magic = p_value; }
	void set_resistance(int p_value) { resistance = p_value; }
	void set_aim(int p_value) { aim = p_value; }
	void set_evasion(int p_value) { evasion = p_value; }
	void set_crit_rate(float p_value) { crit_rate = p_value; }
	void set_crit_damage(float p_value) { crit_damage = p_value; }
	int get_attack() const { return attack; }
	int get_defense() const { return defense; }
	int get_magic() const { return magic; }
	int get_resistance() const { return resistance; }
	int get_aim() const { return aim; }
	int get_evasion() const { return evasion; }
	float get_crit_rate() const { return crit_rate; }
	float get_crit_damage() const { return crit_damage; }

	virtual Dictionary to_dict() const override;
	virtual void from_dict(const Dictionary &p_dict) override;
	virtual Ref<CombatComponent> clone() const override;
};

#endif // COMBAT_ATTR_COMPONENT_H
