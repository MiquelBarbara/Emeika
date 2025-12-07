#include "Globals.h"
#include "CameraModule.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Time.hpp"
#include "Application.h"
#include "D3D12Module.h"


CameraModule::~CameraModule()
{
	_inputModule = nullptr;
}

bool CameraModule::init()
{
	CalculateViewMatrix();
	return true;
}

bool CameraModule::postInit()
{
	AspectRatio(app->GetD3D12Module()->GetWindow()->Width() / app->GetD3D12Module()->GetWindow()->Height());
	_inputModule = app->GetInputModule();
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

	_forward = newTarget - newPos;
	_forward.Normalize();

	_right = _forward.Cross(_up);
	_right.Normalize();

	//Mouse look
	float deltaX = 0.0f;
	float deltaY = 0.0f;

	//Zoom in and out
	float wheel = 0.0f;
	_inputModule->GetMouseWheel(wheel);
	if (wheel != 0) {
		newPos += _forward * wheel * Time::deltaTime();
		newTarget *= _forward * wheel * Time::deltaTime();
	}


	if (_inputModule->IsRightMouseDown()) {
		_inputModule->GetMouseDelta(deltaX, deltaY);

		Quaternion yaw = Quaternion::CreateFromAxisAngle(_up, deltaX * _sensitivity);
		Quaternion pitch = Quaternion::CreateFromAxisAngle(_right, deltaY * _sensitivity);
		_rotation = yaw * pitch;

		Vector3 direction = newTarget - newPos;
		direction = Vector3::Transform(direction, Matrix::CreateFromQuaternion(_rotation));
		newTarget = newPos + direction;

		_forward = newTarget - newPos;
		_forward.Normalize();

		_right = _forward.Cross(_up);
		_right.Normalize();

		//Move forward or backward
		if (_inputModule->IsKeyDown(Keyboard::W)) {
			newPos += _forward * Time::deltaTime();
			newTarget += _forward * Time::deltaTime();
		}
		if (_inputModule->IsKeyDown(Keyboard::S)) {
			newPos -= _forward * Time::deltaTime();
			newTarget -= _forward * Time::deltaTime();
		}

		//Up or down
		if (_inputModule->IsKeyDown(Keyboard::Q)) {
			newPos.y += 1.0f * Time::deltaTime();
			newTarget.y += 1.0f * Time::deltaTime();
		}

		if (_inputModule->IsKeyDown(Keyboard::E)) {
			newPos.y -= 1.0f * Time::deltaTime();
			newTarget.y -= 1.0f * Time::deltaTime();
		}

		//Left or Right
		if (_inputModule->IsKeyDown(Keyboard::A)) {
			newPos -= _right * Time::deltaTime();
			newTarget -= _right * Time::deltaTime();
		}
		if (_inputModule->IsKeyDown(Keyboard::D)) {
			newPos += _right * Time::deltaTime();
			newTarget += _right * Time::deltaTime();
		}
	}

	//Focus the camera to the geometry
	if (_inputModule->IsKeyDown(Keyboard::F)) {
		Matrix* model = app->GetD3D12Module()->GetModelMatrix();
		newTarget = Vector3::Transform(Vector3::Zero, *model);
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

void CameraModule::Resize()
{
	unsigned int width, height;
	app->GetD3D12Module()->GetWindow()->GetWindowSize(width, height);
	if (width > height) {
		AspectRatio(width / height);
	}
}

void CameraModule::CalculateProjectionMatrix()
{
	_proj = Matrix::CreatePerspectiveFieldOfView(_fov, _aspectRatio, _nearPlane, _farPlane);
}

void CameraModule::CalculateViewMatrix() {
		
	_view = Matrix::CreateLookAt(_eye, _target, _up);
}
