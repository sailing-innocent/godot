#include "skill_context.h"

#include "state/battle_state.h"

#include "core/object/class_db.h"

void SkillContext::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_state", "state"), &SkillContext::set_state);
	ClassDB::bind_method(D_METHOD("get_state"), &SkillContext::get_state);
	ClassDB::bind_method(D_METHOD("set_caster_id", "caster_id"), &SkillContext::set_caster_id);
	ClassDB::bind_method(D_METHOD("get_caster_id"), &SkillContext::get_caster_id);
	ClassDB::bind_method(D_METHOD("set_target_id", "target_id"), &SkillContext::set_target_id);
	ClassDB::bind_method(D_METHOD("get_target_id"), &SkillContext::get_target_id);
	ClassDB::bind_method(D_METHOD("set_target_coord", "target_coord"), &SkillContext::set_target_coord);
	ClassDB::bind_method(D_METHOD("get_target_coord"), &SkillContext::get_target_coord);
	ClassDB::bind_method(D_METHOD("set_skill", "skill"), &SkillContext::set_skill);
	ClassDB::bind_method(D_METHOD("get_skill"), &SkillContext::get_skill);
	ClassDB::bind_method(D_METHOD("set_rules", "rules"), &SkillContext::set_rules);
	ClassDB::bind_method(D_METHOD("get_rules"), &SkillContext::get_rules);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "state", PROPERTY_HINT_RESOURCE_TYPE, "BattleState"), "set_state", "get_state");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "caster_id"), "set_caster_id", "get_caster_id");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "target_id"), "set_target_id", "get_target_id");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2I, "target_coord"), "set_target_coord", "get_target_coord");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "skill", PROPERTY_HINT_RESOURCE_TYPE, "SkillDef"), "set_skill", "get_skill");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "rules", PROPERTY_HINT_RESOURCE_TYPE, "RuleSet"), "set_rules", "get_rules");
}
