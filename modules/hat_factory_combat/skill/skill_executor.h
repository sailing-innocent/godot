#ifndef SKILL_EXECUTOR_H
#define SKILL_EXECUTOR_H

#include "core/object/ref_counted.h"
#include "core/variant/typed_array.h"

class HitResult;
class SkillContext;

class SkillExecutor : public RefCounted {
	GDCLASS(SkillExecutor, RefCounted)

protected:
	static void _bind_methods();

public:
	TypedArray<HitResult> execute(const Ref<SkillContext> &p_ctx) const;
};

#endif // SKILL_EXECUTOR_H
