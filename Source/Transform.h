#pragma once
#include "Globals.h"

/// <summary>
/// Represents a 3D transformation including position, rotation, and scale.
/// </summary>
class Transform
{
public:
	Transform() = default;
	~Transform() = default;

	const Vector3& GetPosition() const { return position; }
	void SetPosition(const Vector3& pos) { position = pos; }

	const Quaternion& GetRotation() const { return rotation; }
	void SetRotation(const Quaternion& rot) { rotation = rot; }

	const Vector3& GetScale() const { return scale; }
	void SetScale(const Vector3& scl) { scale = scl; }

	Matrix& GetWorldMatrix()
	{
		worldMatrix = Matrix::CreateScale(scale) *
			Matrix::CreateFromQuaternion(rotation) *
			Matrix::CreateTranslation(position);
		return worldMatrix;
	}

	void SetWorldMatrix(Matrix& matrix) {
		matrix.Decompose(scale, rotation, position);
	}
private:
	Vector3 position = Vector3::Zero;
	Quaternion rotation = Quaternion::Identity;
	Vector3 scale = Vector3::One;

	Matrix worldMatrix = Matrix::Identity;
};

