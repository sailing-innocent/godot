#ifndef RULE_SET_H
#define RULE_SET_H

#include "core/io/resource.h"
#include "core/object/ref_counted.h"
#include "core/string/string_name.h"
#include "core/variant/dictionary.h"

class CombatEntity;
class SkillDef;

class RuleSet : public Resource {
	GDCLASS(RuleSet, Resource)

	int max_rounds = 99;
	float time_limit_seconds = 0.0f;
	String fallback_formula = "(atk - def) * 1.0";
	Dictionary skill_library; // StringName -> Ref<SkillDef>

	// --- Terrain combat modifiers (MVP) ---
	// Damage multiplier granted per height level the attacker stands above the
	// defender. Negative diff (attacker below) reduces damage symmetrically.
	float height_damage_step = 0.15f;
	// Cap on |attacker_height - defender_height| that contributes to the bonus.
	int max_height_bonus_levels = 2;
	// Extra evasion granted to a defender standing on concealment terrain.
	int conceal_evasion_bonus = 20;
	// Terrain id that counts as concealment (grants the evasion bonus).
	StringName conceal_terrain_id = "grass";
	// When false (legacy), every attack lands; when true, aim/evasion decide
	// hit/miss and a miss deals no damage.
	bool use_accuracy_system = false;

	// --- Action energy regen per turn start (MVP, R6) ---
	// Only consumed when use_terrain_energy_regen is enabled; otherwise turns
	// refill AP to max (legacy behavior).
	bool use_terrain_energy_regen = false;
	int energy_recovery_base = 2;      // plains / road / anything else
	int energy_recovery_conceal = 1;   // concealment terrain (grass)
	int energy_recovery_water = 0;     // water / shallow water
	int energy_recovery_highland = 3;  // cell height >= highland_min_height
	int highland_min_height = 1;

protected:
	static void _bind_methods();

public:
	void set_max_rounds(int p_value) { max_rounds = p_value; }
	int get_max_rounds() const { return max_rounds; }

	void set_time_limit_seconds(float p_value) { time_limit_seconds = p_value; }
	float get_time_limit_seconds() const { return time_limit_seconds; }

	void set_fallback_formula(const String &p_value) { fallback_formula = p_value; }
	String get_fallback_formula() const { return fallback_formula; }

	void set_skill_library(const Dictionary &p_value) { skill_library = p_value; }
	Dictionary get_skill_library() const { return skill_library; }
	void register_skill(const StringName &p_skill_id, const Ref<SkillDef> &p_def);
	Ref<SkillDef> get_skill_def(const StringName &p_skill_id) const;

	// Terrain combat / energy helpers.
	void set_height_damage_step(float p_value) { height_damage_step = p_value; }
	float get_height_damage_step() const { return height_damage_step; }
	void set_max_height_bonus_levels(int p_value) { max_height_bonus_levels = p_value; }
	int get_max_height_bonus_levels() const { return max_height_bonus_levels; }
	void set_conceal_evasion_bonus(int p_value) { conceal_evasion_bonus = p_value; }
	int get_conceal_evasion_bonus() const { return conceal_evasion_bonus; }
	void set_conceal_terrain_id(const StringName &p_value) { conceal_terrain_id = p_value; }
	StringName get_conceal_terrain_id() const { return conceal_terrain_id; }
	void set_use_accuracy_system(bool p_value) { use_accuracy_system = p_value; }
	bool get_use_accuracy_system() const { return use_accuracy_system; }

	void set_use_terrain_energy_regen(bool p_value) { use_terrain_energy_regen = p_value; }
	bool get_use_terrain_energy_regen() const { return use_terrain_energy_regen; }
	void set_energy_recovery_base(int p_value) { energy_recovery_base = p_value; }
	int get_energy_recovery_base() const { return energy_recovery_base; }
	void set_energy_recovery_conceal(int p_value) { energy_recovery_conceal = p_value; }
	int get_energy_recovery_conceal() const { return energy_recovery_conceal; }
	void set_energy_recovery_water(int p_value) { energy_recovery_water = p_value; }
	int get_energy_recovery_water() const { return energy_recovery_water; }
	void set_energy_recovery_highland(int p_value) { energy_recovery_highland = p_value; }
	int get_energy_recovery_highland() const { return energy_recovery_highland; }
	void set_highland_min_height(int p_value) { highland_min_height = p_value; }
	int get_highland_min_height() const { return highland_min_height; }

	// True when the given terrain id grants concealment (evasion bonus).
	bool is_conceal_terrain(const StringName &p_terrain_id) const;
	// Energy regen for a unit standing on (terrain, cell height) at turn start.
	int get_energy_regen_for(const StringName &p_terrain_id, int p_height) const;
	// Height-damage multiplier for a given attacker/defender height delta.
	float get_height_damage_multiplier(int p_height_diff) const;

	int evaluate_damage(const Ref<CombatEntity> &p_attacker, const Ref<CombatEntity> &p_defender, int p_base_damage) const;

	Dictionary to_dict() const;
	static Ref<RuleSet> from_dict(const Dictionary &p_dict);
	Ref<RuleSet> clone() const;
};

#endif // RULE_SET_H
