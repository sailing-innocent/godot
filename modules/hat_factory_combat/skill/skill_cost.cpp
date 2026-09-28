#include "skill_cost.h"

#include "core/object/class_db.h"

void SkillCost::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_cost_type", "cost_type"), &SkillCost::set_cost_type);
	ClassDB::bind_method(D_METHOD("get_cost_type"), &SkillCost::get_cost_type);
	ClassDB::bind_method(D_METHOD("set_item_id", "item_id"), &SkillCost::set_item_id);
	ClassDB::bind_method(D_METHOD("get_item_id"), &SkillCost::get_item_id);
	ClassDB::bind_method(D_METHOD("set_amount", "amount"), &SkillCost::set_amount);
	ClassDB::bind_method(D_METHOD("get_amount"), &SkillCost::get_amount);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "cost_type"), "set_cost_type", "get_cost_type");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "item_id"), "set_item_id", "get_item_id");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "amount"), "set_amount", "get_amount");
}
