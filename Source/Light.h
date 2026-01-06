#pragma once
#include "Component.h"


class Light: public Component
{
public:
	enum Mode {
		REALTIME,
		MIXED,
		BAKED
	};

	enum Type {
		DIRECTIONAL,
		POINT,
		SPOT, 
		AREA
	};

	Light(Type type = Type::DIRECTIONAL) : type(type){}
	constexpr Vector3& GetColor() { return color; }
	constexpr Vector3& GetAmbientColor() { return ambientColor; }
private:
	Type type;
	Vector3 ambientColor = Vector3::One * (0.1f);
	Vector3 color = Vector3(1.0f, 1.0f, 1.0f);
};

