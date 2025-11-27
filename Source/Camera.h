#pragma once

enum CameraProjectionType
{
	Perspective,
	Orthographic
};

/// <summary>
/// This class represents a simple camera with view and projection matrices.
/// The Camera class allows setting the view matrix based on eye, target, and up vectors,
/// this may be extended to CameraScript for the ECS pattern.
/// </summary>
class Camera
{
	public:
	Camera();
	~Camera();

	void SetViewMatrix(const Vector3& eye, const Vector3& target, const Vector3& up);
	void SetPerspectiveProjection(float fovY, float aspectRatio, float nearZ, float farZ);
	void SetOrthographicProjection(float viewWidth, float viewHeight, float nearZ, float farZ);

	const Vector3& GetEye() const { return eye; }
	const Vector3& GetTarget() const { return target; }
	const Vector3& GetUp() const { return up; }

	const Matrix& GetViewMatrix() const { return viewMatrix; }
	const Matrix& GetProjectionMatrix() const { return projectionMatrix; }
	CameraProjectionType GetProjectionType() const { return projectionType; }
private:
	Matrix viewMatrix = Matrix::Identity;
	CameraProjectionType projectionType = CameraProjectionType::Perspective;
	Matrix projectionMatrix = Matrix::Identity;

	Vector3 eye = Vector3(0.0f, 10.f, 10.f);
	Vector3 target = Vector3::Zero;
	Vector3 up = Vector3::Up;
};

