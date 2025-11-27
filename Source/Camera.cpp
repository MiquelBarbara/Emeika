#include "Globals.h"
#include "Camera.h"

Camera::Camera()
{
	projectionType = CameraProjectionType::Perspective;
	viewMatrix = Matrix::Identity;
	projectionMatrix = Matrix::Identity;
}

Camera::~Camera()
{
}

void Camera::SetViewMatrix(const Vector3& eye, const Vector3& target, const Vector3& up)
{
	this->eye = eye;
	this->target = target;
	this->up = up;
	viewMatrix = Matrix::CreateLookAt(eye, target, up);
}

void Camera::SetPerspectiveProjection(float fovY, float aspectRatio, float nearZ, float farZ)
{
	projectionType = CameraProjectionType::Perspective;
	projectionMatrix = Matrix::CreatePerspectiveFieldOfView(fovY, aspectRatio, nearZ, farZ);
}

void Camera::SetOrthographicProjection(float viewWidth, float viewHeight, float nearZ, float farZ)
{
	projectionType = CameraProjectionType::Orthographic;
	projectionMatrix = Matrix::CreateOrthographic(viewWidth,  viewHeight, nearZ, farZ);
}
