#include "rule_set.h"

#include "combat_entity.h"
#include "skill/skill_def.h"
#include "core/math/math_funcs.h"
#include "components/combat_attr_component.h"
#include "core/math/expression.h"
#include "core/object/class_db.h"
#include "core/io/resource_loader.h"
#include "core/variant/typed_array.h"

void RuleSet::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_max_rounds", "max_rounds"), &RuleSet::set_max_rounds);
	ClassDB::bind_method(D_METHOD("get_max_rounds"), &RuleSet::get_max_rounds);
	ClassDB::bind_method(D_METHOD("set_time_limit_seconds", "time_limit_seconds"), &RuleSet::set_time_limit_seconds);
	ClassDB::bind_method(D_METHOD("get_time_limit_seconds"), &RuleSet::get_time_limit_seconds);
	ClassDB::bind_method(D_METHOD("set_fallback_formula", "fallback_formula"), &RuleSet::set_fallback_formula);
	ClassDB::bind_method(D_METHOD("get_fallback_formula"), &RuleSet::get_fallback_formula);
	ClassDB::bind_method(D_METHOD("set_skill_library", "skill_library"), &RuleSet::set_skill_library);
	ClassDB::bind_method(D_METHOD("get_skill_library"), &RuleSet::get_skill_library);
	ClassDB::bind_method(D_METHOD("register_skill", "skill_id", "def"), &RuleSet::register_skill);
	ClassDB::bind_method(D_METHOD("get_skill_def", "skill_id"), &RuleSet::get_skill_def);

	ClassDB::bind_method(D_METHOD("set_height_damage_step", "height_damage_step"), &RuleSet::set_height_damage_step);
	ClassDB::bind_method(D_METHOD("get_height_damage_step"), &RuleSet::get_height_damage_step);
	ClassDB::bind_method(D_METHOD("set_max_height_bonus_levels", "max_height_bonus_levels"), &RuleSet::set_max_height_bonus_levels);
	ClassDB::bind_method(D_METHOD("get_max_height_bonus_levels"), &RuleSet::get_max_height_bonus_levels);
	ClassDB::bind_method(D_METHOD("set_conceal_evasion_bonus", "conceal_evasion_bonus"), &RuleSet::set_conceal_evasion_bonus);
	ClassDB::bind_method(D_METHOD("get_conceal_evasion_bonus"), &RuleSet::get_conceal_evasion_bonus);
	ClassDB::bind_method(D_METHOD("set_conceal_terrain_id", "conceal_terrain_id"), &RuleSet::set_conceal_terrain_id);
	ClassDB::bind_method(D_METHOD("get_conceal_terrain_id"), &RuleSet::get_conceal_terrain_id);
	ClassDB::bind_method(D_METHOD("set_use_accuracy_system", "use_accuracy_system"), &RuleSet::set_use_accuracy_system);
	ClassDB::bind_method(D_METHOD("get_use_accuracy_system"), &RuleSet::get_use_accuracy_system);

	ClassDB::bind_method(D_METHOD("set_use_terrain_energy_regen", "use_terrain_energy_regen"), &RuleSet::set_use_terrain_energy_regen);
	ClassDB::bind_method(D_METHOD("get_use_terrain_energy_regen"), &RuleSet::get_use_terrain_energy_regen);
	ClassDB::bind_method(D_METHOD("set_energy_recovery_base", "energy_recovery_base"), &RuleSet::set_energy_recovery_base);
	ClassDB::bind_method(D_METHOD("get_energy_recovery_base"), &RuleSet::get_energy_recovery_base);
	ClassDB::bind_method(D_METHOD("set_energy_recovery_conceal", "energy_recovery_conceal"), &RuleSet::set_energy_recovery_conceal);
	ClassDB::bind_method(D_METHOD("get_energy_recovery_conceal"), &RuleSet::get_energy_recovery_conceal);
	ClassDB::bind_method(D_METHOD("set_energy_recovery_water", "energy_recovery_water"), &RuleSet::set_energy_recovery_water);
	ClassDB::bind_method(D_METHOD("get_energy_recovery_water"), &RuleSet::get_energy_recovery_water);
	ClassDB::bind_method(D_METHOD("set_energy_recovery_highland", "energy_recovery_highland"), &RuleSet::set_energy_recovery_highland);
	ClassDB::bind_method(D_METHOD("get_energy_recovery_highland"), &RuleSet::get_energy_recovery_highland);
	ClassDB::bind_method(D_METHOD("set_highland_min_height", "highland_min_height"), &RuleSet::set_highland_min_height);
	ClassDB::bind_method(D_METHOD("get_highland_min_height"), &RuleSet::get_highland_min_height);

	ClassDB::bind_method(D_METHOD("is_conceal_terrain", "terrain_id"), &RuleSet::is_conceal_terrain);
	ClassDB::bind_method(D_METHOD("get_energy_regen_for", "terrain_id", "height"), &RuleSet::get_energy_regen_for);
	ClassDB::bind_method(D_METHOD("get_height_damage_multiplier", "height_diff"), &RuleSet::get_height_damage_multiplier);

	ClassDB::bind_method(D_METHOD("evaluate_damage", "attacker", "defender", "base_damage"), &RuleSet::evaluate_damage);
	ClassDB::bind_method(D_METHOD("to_dict"), &RuleSet::to_dict);
	ClassDB::bind_static_method("RuleSet", D_METHOD("from_dict", "dict"), &RuleSet::from_dict);
	ClassDB::bind_method(D_METHOD("clone"), &RuleSet::clone);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_rounds"), "set_max_rounds", "get_max_rounds");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "time_limit_seconds"), "set_time_limit_seconds", "get_time_limit_seconds");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "fallback_formula"), "set_fallback_formula", "get_fallback_formula");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "skill_library"), "set_skill_library", "get_skill_library");

	ADD_GROUP("Terrain Combat", "");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "height_damage_step"), "set_height_damage_step", "get_height_damage_step");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_height_bonus_levels"), "set_max_height_bonus_levels", "get_max_height_bonus_levels");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "conceal_evasion_bonus"), "set_conceal_evasion_bonus", "get_conceal_evasion_bonus");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "conceal_terrain_id"), "set_conceal_terrain_id", "get_conceal_terrain_id");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_accuracy_system"), "set_use_accuracy_system", "get_use_accuracy_system");

	ADD_GROUP("Action Energy", "");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_terrain_energy_regen"), "set_use_terrain_energy_regen", "get_use_terrain_energy_regen");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "energy_recovery_base"), "set_energy_recovery_base", "get_energy_recovery_base");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "energy_recovery_conceal"), "set_energy_recovery_conceal", "get_energy_recovery_conceal");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "energy_recovery_water"), "set_energy_recovery_water", "get_energy_recovery_water");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "energy_recovery_highland"), "set_energy_recovery_highland", "get_energy_recovery_highland");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "highland_min_height"), "set_highland_min_height", "get_highland_min_height");
}

bool RuleSet::is_conceal_terrain(const StringName &p_terrain_id) const {
	return p_terrain_id != StringName() && p_terrain_id == conceal_terrain_id;
}

int RuleSet::get_energy_regen_for(const StringName &p_terrain_id, int p_height) const {
	if (p_height >= highland_min_height) {
		return energy_recovery_highland;
	}
	if (is_conceal_terrain(p_terrain_id)) {
		return energy_recovery_conceal;
	}
	if (p_terrain_id == StringName("water") || p_terrain_id == StringName("shallow_water")) {
		return energy_recovery_water;
	}
	return energy_recovery_base;
}

float RuleSet::get_height_damage_multiplier(int p_height_diff) const {
	int clamped = CLAMP(p_height_diff, -max_height_bonus_levels, max_height_bonus_levels);
	return 1.0f + height_damage_step * (float)clamped;
}

void RuleSet::register_skill(const StringName &p_skill_id, const Ref<SkillDef> &p_def) {
	if (p_skill_id == StringName() || p_def.is_null()) {
		return;
	}
	skill_library[p_skill_id] = p_def;
}

Ref<SkillDef> RuleSet::get_skill_def(const StringName &p_skill_id) const {
	if (p_skill_id == StringName() || !skill_library.has(p_skill_id)) {
		return Ref<SkillDef>();
	}
	return skill_library[p_skill_id];
}

int RuleSet::evaluate_damage(const Ref<CombatEntity> &p_attacker, const Ref<CombatEntity> &p_defender, int p_base_damage) const {
	int atk = 0;
	int def = 0;
	int magic = 0;
	int resistance = 0;

	if (p_attacker.is_valid() && p_attacker->has_component(StringName("CombatAttr"))) {
		Ref<CombatAttrComponent> attr = p_attacker->get_component(StringName("CombatAttr"));
		if (attr.is_valid()) {
			atk = attr->get_attack();
			magic = attr->get_magic();
		}
	}
	if (p_defender.is_valid() && p_defender->has_component(StringName("CombatAttr"))) {
		Ref<CombatAttrComponent> attr = p_defender->get_component(StringName("CombatAttr"));
		if (attr.is_valid()) {
			def = attr->get_defense();
			resistance = attr->get_resistance();
		}
	}

	Expression expr;
	String expr_str = fallback_formula.is_empty() ? String("(atk - def) * 1.0") : fallback_formula;
	Vector<String> input_names;
	input_names.push_back("atk");
	input_names.push_back("def");
	input_names.push_back("base_damage");
	input_names.push_back("magic");
	input_names.push_back("resistance");

	Error err = expr.parse(expr_str, input_names);
	if (err != OK) {
		return MAX(0, atk - def);
	}

	Array inputs;
	inputs.push_back(atk);
	inputs.push_back(def);
	inputs.push_back(p_base_damage);
	inputs.push_back(magic);
	inputs.push_back(resistance);

	bool expr_exec_failed = false;
	Variant result = expr.execute(inputs, nullptr, false, &expr_exec_failed);
	if (expr_exec_failed || result.get_type() != Variant::FLOAT && result.get_type() != Variant::INT) {
		return MAX(0, atk - def);
	}
	return MAX(0, (int)result);
}

Dictionary RuleSet::to_dict() const {
	Dictionary d;
	d["max_rounds"] = max_rounds;
	d["time_limit_seconds"] = time_limit_seconds;
	d["fallback_formula"] = fallback_formula;
	d["height_damage_step"] = height_damage_step;
	d["max_height_bonus_levels"] = max_height_bonus_levels;
	d["conceal_evasion_bonus"] = conceal_evasion_bonus;
	d["conceal_terrain_id"] = conceal_terrain_id;
	d["use_accuracy_system"] = use_accuracy_system;
	d["use_terrain_energy_regen"] = use_terrain_energy_regen;
	d["energy_recovery_base"] = energy_recovery_base;
	d["energy_recovery_conceal"] = energy_recovery_conceal;
	d["energy_recovery_water"] = energy_recovery_water;
	d["energy_recovery_highland"] = energy_recovery_highland;
	d["highland_min_height"] = highland_min_height;
	Dictionary paths;
	Dictionary versions;
	Array ids = skill_library.keys();
	for (int i = 0; i < ids.size(); i++) {
		Ref<SkillDef> def = skill_library[ids[i]];
		if (def.is_valid() && !def->get_path().is_empty()) {
			paths[ids[i]] = def->get_path();
			versions[ids[i]] = def->get_definition_version();
		}
	}
	d["skill_paths"] = paths;
	d["skill_versions"] = versions;
	return d;
}

Ref<RuleSet> RuleSet::from_dict(const Dictionary &p_dict) {
	Ref<RuleSet> rs;
	rs.instantiate();
	if (p_dict.has("max_rounds")) rs->max_rounds = p_dict["max_rounds"];
	if (p_dict.has("time_limit_seconds")) rs->time_limit_seconds = p_dict["time_limit_seconds"];
	if (p_dict.has("fallback_formula")) rs->fallback_formula = p_dict["fallback_formula"];
	if (p_dict.has("height_damage_step")) rs->height_damage_step = p_dict["height_damage_step"];
	if (p_dict.has("max_height_bonus_levels")) rs->max_height_bonus_levels = p_dict["max_height_bonus_levels"];
	if (p_dict.has("conceal_evasion_bonus")) rs->conceal_evasion_bonus = p_dict["conceal_evasion_bonus"];
	if (p_dict.has("conceal_terrain_id")) rs->conceal_terrain_id = p_dict["conceal_terrain_id"];
	if (p_dict.has("use_accuracy_system")) rs->use_accuracy_system = p_dict["use_accuracy_system"];
	if (p_dict.has("use_terrain_energy_regen")) rs->use_terrain_energy_regen = p_dict["use_terrain_energy_regen"];
	if (p_dict.has("energy_recovery_base")) rs->energy_recovery_base = p_dict["energy_recovery_base"];
	if (p_dict.has("energy_recovery_conceal")) rs->energy_recovery_conceal = p_dict["energy_recovery_conceal"];
	if (p_dict.has("energy_recovery_water")) rs->energy_recovery_water = p_dict["energy_recovery_water"];
	if (p_dict.has("energy_recovery_highland")) rs->energy_recovery_highland = p_dict["energy_recovery_highland"];
	if (p_dict.has("highland_min_height")) rs->highland_min_height = p_dict["highland_min_height"];
	if (p_dict.has("skill_paths")) {
		Dictionary paths = p_dict["skill_paths"];
		Dictionary versions = p_dict.get("skill_versions", Dictionary());
		Array ids = paths.keys();
		for (int i = 0; i < ids.size(); i++) {
			StringName id = ids[i];
			Ref<SkillDef> def = ResourceLoader::load(paths[id]);
			if (def.is_valid() && def->get_skill_id() == id &&
				(!versions.has(id) || def->get_definition_version() == int(versions[id]))) {
				rs->register_skill(id, def);
			}
		}
	}
	return rs;
}

Ref<RuleSet> RuleSet::clone() const {
	Ref<RuleSet> rs;
	rs.instantiate();
	rs->max_rounds = max_rounds;
	rs->time_limit_seconds = time_limit_seconds;
	rs->fallback_formula = fallback_formula;
	rs->height_damage_step = height_damage_step;
	rs->max_height_bonus_levels = max_height_bonus_levels;
	rs->conceal_evasion_bonus = conceal_evasion_bonus;
	rs->conceal_terrain_id = conceal_terrain_id;
	rs->use_accuracy_system = use_accuracy_system;
	rs->use_terrain_energy_regen = use_terrain_energy_regen;
	rs->energy_recovery_base = energy_recovery_base;
	rs->energy_recovery_conceal = energy_recovery_conceal;
	rs->energy_recovery_water = energy_recovery_water;
	rs->energy_recovery_highland = energy_recovery_highland;
	rs->highland_min_height = highland_min_height;
	// Skill library entries are shared config resources; a shallow copy is intentional.
	rs->skill_library = skill_library.duplicate();
	return rs;
}
