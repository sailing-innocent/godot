#ifndef SKILL_COST_H
#define SKILL_COST_H

#include "core/io/resource.h"
#include "core/string/string_name.h"

class SkillCost : public Resource {
	GDCLASS(SkillCost, Resource)

	int cost_type = 0; // 0=ap, 1=mp, 2=hp, 3=item
	StringName item_id;
	int amount = 0;

protected:
	static void _bind_methods();

public:
	void set_cost_type(int p_value) { cost_type = p_value; }
	int get_cost_type() const { return cost_type; }

	void set_item_id(const StringName &p_value) { item_id = p_value; }
	StringName get_item_id() const { return item_id; }

	void set_amount(int p_value) { amount = p_value; }
	int get_amount() const { return amount; }
};

#endif // SKILL_COST_H
