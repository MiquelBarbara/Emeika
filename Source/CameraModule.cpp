#include "Globals.h"
#include "CameraModule.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Time.hpp"

bool CameraModule::init()
{
	_view = Matrix::CreateLookAt(_eye, _target, _up);
	return true;
}

void CameraModule::update()
{
	if (_isDirty) {
		CalculateProjectionMatrix();
		_isDirty = false;
	}

	//Move forward or backward
	if (Keyboard::W) {

	}
	if (Keyboard::S) {

	}

	//Up or down
	if (Keyboard::Q) {
		SetPosition(Vector3(_eye.x, _eye.y + 1.0f * Time::deltaTime(), _eye.z));
	}
	if (Keyboard::E) {
		SetPosition(Vector3(_eye.x, _eye.y - 1.0f * Time::deltaTime(), _eye.z));
	}

	//Left or Right
	if (Keyboard::A) {

	}
	if (Keyboard::D) {

	}
}

void CameraModule::SetFOV(const float fov)
{
	_fov = fov;
	_isDirty = true;
}

void CameraModule::AspectRatio(const float aspectRatio)
{
	_aspectRatio = aspectRatio;
	_isDirty = true;
}

void CameraModule::SetPlaneDistances(const float nearPlane, const float farPlane)
{
	_nearPlane = nearPlane;
	_farPlane = farPlane;
	_isDirty = true;
}

void CameraModule::SetPosition(const Vector3& position)
{
	_eye = position;
	_isDirty = true;
}

void CameraModule::SetOrientation(const Vector3& orientation)
{
	_target = orientation;
	_isDirty = true;
}

void CameraModule::LookAt(const Vector3& lookAt)
{
	_eye = lookAt;
	_view = Matrix::CreateLookAt(lookAt, Vector3::Zero, Vector3::Up);
}

void CameraModule::CalculateProjectionMatrix()
{
	_proj = Matrix::CreatePerspectiveFieldOfView(_fov, _aspectRatio, _nearPlane, _farPlane);
}
