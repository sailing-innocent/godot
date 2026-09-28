#ifndef COMBAT_COMPONENT_H
#define COMBAT_COMPONENT_H

#include "core/object/ref_counted.h"
#include "core/string/string_name.h"
#include "core/variant/dictionary.h"

class CombatComponent : public RefCounted {
	GDCLASS(CombatComponent, RefCounted)

protected:
	static void _bind_methods();

public:
	virtual StringName get_component_name() const;
	virtual Dictionary to_dict() const;
	virtual void from_dict(const Dictionary &p_dict);
	virtual Ref<CombatComponent> clone() const;
};

#endif // COMBAT_COMPONENT_H
