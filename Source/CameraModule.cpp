#include "Globals.h"
#include "CameraModule.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Time.hpp"
#include "Application.h"
#include "D3D12Module.h"


CameraModule::~CameraModule()
{
	inputModule = nullptr;
}

bool CameraModule::init()
{
	CalculateViewMatrix();
	return true;
}

bool CameraModule::postInit()
{
	AspectRatio(app->getD3D12Module()->GetWindow()->Width() / app->getD3D12Module()->GetWindow()->Height());
	inputModule = app->getInputModule();
	return true;
}

void CameraModule::update()
{
	if (_isDirty) {
		CalculateViewMatrix();
		CalculateProjectionMatrix();
		_isDirty = false;
	}

	Vector3 newPos = _eye;
	Vector3 newTarget = _target;

	//Mouse look
	float deltaX = 0.0f;
	float deltaY = 0.0f;

	if (inputModule->IsRightMouseDown()) {
		inputModule->GetMouseDelta(deltaX, deltaY);

		Vector3 forward = (newTarget - newPos);
		forward.Normalize();

		Vector3 right = forward.Cross(_up);
		right.Normalize();

		Quaternion yaw = Quaternion::CreateFromAxisAngle(_up, deltaX * sensitivity);
		Quaternion pitch = Quaternion::CreateFromAxisAngle(_right, deltaY * sensitivity);
		_rotation = yaw * pitch;

		Vector3 direction = newTarget - newPos;
		direction = Vector3::Transform(direction, Matrix::CreateFromQuaternion(_rotation));
		newTarget = newPos + direction;

		_forward = newTarget - newPos;
		_forward.Normalize();

		_right = _forward.Cross(_up);
		_right.Normalize();

		//Move forward or backward
		if (inputModule->IsKeyDown(Keyboard::W)) {
			newPos += _forward * Time::deltaTime();
			newTarget += _forward * Time::deltaTime();
		}
		if (inputModule->IsKeyDown(Keyboard::S)) {
			newPos -= _forward * Time::deltaTime();
			newTarget -= _forward * Time::deltaTime();
		}

		//Up or down
		if (inputModule->IsKeyDown(Keyboard::Q)) {
			newPos.y += 1.0f * Time::deltaTime();
			newTarget.y += 1.0f * Time::deltaTime();
		}

		if (inputModule->IsKeyDown(Keyboard::E)) {
			newPos.y -= 1.0f * Time::deltaTime();
			newTarget.y -= 1.0f * Time::deltaTime();
		}

		//Left or Right
		if (inputModule->IsKeyDown(Keyboard::A)) {
			newPos -= _right * Time::deltaTime();
			newTarget -= _right * Time::deltaTime();
		}
		if (inputModule->IsKeyDown(Keyboard::D)) {
			newPos += _right * Time::deltaTime();
			newTarget += _right * Time::deltaTime();
		}
	}

	SetPosition(newPos);
	SetOrientation(newTarget);
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
	_isDirty = true;
}

void CameraModule::CalculateProjectionMatrix()
{
	_proj = Matrix::CreatePerspectiveFieldOfView(_fov, _aspectRatio, _nearPlane, _farPlane);
}

void CameraModule::CalculateViewMatrix() {
		
	_view = Matrix::CreateLookAt(_eye, _target, _up);
}
