#ifndef TRANSFORM_COMPONENT_H
#define TRANSFORM_COMPONENT_H

#include "combat_component.h"

class TransformComponent : public CombatComponent {
	GDCLASS(TransformComponent, CombatComponent)

	Vector2i coord;
	int height = 0;
	float face_angle = 0.0f;

protected:
	static void _bind_methods();

public:
	virtual StringName get_component_name() const override { return StringName("Transform"); }

	void set_coord(Vector2i p_value) { coord = p_value; }
	void set_height(int p_value) { height = p_value; }
	void set_face_angle(float p_value) { face_angle = p_value; }
	Vector2i get_coord() const { return coord; }
	int get_height() const { return height; }
	float get_face_angle() const { return face_angle; }

	virtual Dictionary to_dict() const override;
	virtual void from_dict(const Dictionary &p_dict) override;
	virtual Ref<CombatComponent> clone() const override;
};

#endif // TRANSFORM_COMPONENT_H
