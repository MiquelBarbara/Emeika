#include "Globals.h"
#include "CameraModule.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "TimeModule.h"
#include "Application.h"
#include "D3D12Module.h"
#include "EditorModule.h"
#include "SceneEditor.h"


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
	_inputModule = app->GetInputModule();
	return true;
}

void CameraModule::UpdateAxes(const Vector3& pos, const Vector3& target)
{
	_forward = target - pos;
	_forward.Normalize();

	_right = _forward.Cross(_up);
	_right.Normalize();
}

float CameraModule::SpeedBoost(float baseSpeed) const
{
	if (_inputModule->IsKeyDown(Keyboard::LeftShift))
		return baseSpeed * 2.0f;

	return baseSpeed;
}

void CameraModule::Zoom(Vector3& pos, Vector3& target)
{
	float wheel = 0.0f;
	_inputModule->GetMouseWheel(wheel);

	if (wheel == 0.0f)
		return;

	float delta = wheel * app->GetTimeModule()->deltaTime();
	pos += _forward * _speed * delta;
	target += _forward * _speed * delta;
}

void CameraModule::MouseLook(Vector3& pos, Vector3& target)
{
	if (!_inputModule->IsRightMouseDown())
		return;

	float deltaX = 0.0f, deltaY = 0.0f;
	_inputModule->GetMouseDelta(deltaX, deltaY);

	Quaternion yaw = Quaternion::CreateFromAxisAngle(_up, deltaX * _sensitivity);
	Quaternion pitch = Quaternion::CreateFromAxisAngle(_right, deltaY * _sensitivity);
	_rotation = yaw * pitch;

	Vector3 dir = target - pos;
	dir = Vector3::Transform(dir, Matrix::CreateFromQuaternion(_rotation));

	target = pos + dir;

	// Recompute axes after rotation
	UpdateAxes(pos, target);
}

void CameraModule::Movement(Vector3& pos, Vector3& target, float speed)
{
	if (!_inputModule->IsRightMouseDown())
		return;

	float dt = app->GetTimeModule()->deltaTime();

	if (_inputModule->IsKeyDown(Keyboard::W)) {
		pos += _forward * speed * dt;
		target += _forward * speed * dt;
	}
	if (_inputModule->IsKeyDown(Keyboard::S)) {
		pos -= _forward * speed * dt;
		target -= _forward * speed * dt;
	}
	if (_inputModule->IsKeyDown(Keyboard::A)) {
		pos -= _right * speed * dt;
		target -= _right * speed * dt;
	}
	if (_inputModule->IsKeyDown(Keyboard::D)) {
		pos += _right * speed * dt;
		target += _right * speed * dt;
	}
	if (_inputModule->IsKeyDown(Keyboard::Q)) {
		pos.y += speed * dt;
		target.y += speed * dt;
	}
	if (_inputModule->IsKeyDown(Keyboard::E)) {
		pos.y -= speed * dt;
		target.y -= speed * dt;
	}
}

void CameraModule::Focus(Vector3& target)
{
	if (_inputModule->IsKeyDown(Keyboard::F))
	{
		Matrix* model = &app->GetD3D12Module()->GetDuck()->GetWorldMatrix();
		target = Vector3::Transform(Vector3::Zero, *model);
	}
}

void CameraModule::Orbit(Vector3& pos, Vector3& target)
{
	// Orbit is active only when: ALT + Left Mouse
	if (!_inputModule->IsKeyDown(Keyboard::LeftAlt) ||
		!_inputModule->IsLeftMouseDown())
		return;

	float deltaX = 0.0f, deltaY = 0.0f;
	_inputModule->GetMouseDelta(deltaX, deltaY);

	Vector3 offset = pos - target;
	float radius = offset.Length();

	Quaternion yaw = Quaternion::CreateFromAxisAngle(_up, -deltaX * _sensitivity);
	Quaternion pitch = Quaternion::CreateFromAxisAngle(_right, -deltaY * _sensitivity);
	_rotation = yaw * pitch;

	// Rotate offset around target
	offset = Vector3::Transform(offset, Matrix::CreateFromQuaternion(_rotation));

	// Update camera position
	pos = target + offset;

	// Camera now looks at the target
	UpdateAxes(pos, target);
}


void CameraModule::update()
{
	if (_isDirty) {
		CalculateViewMatrix();
		CalculateProjectionMatrix();
		_isDirty = false;
	}

	if (!app->GetEditorModule()->GetSceneEditor()->IsFocused()) {
		return;
	}

	float speed = SpeedBoost(_speed);
	Vector3 newPos = _eye;
	Vector3 newTarget = _target;

	UpdateAxes(newPos, newTarget);

	Zoom(newPos, newTarget);
	MouseLook(newPos, newTarget);
	Orbit(newPos, newTarget);
	Movement(newPos, newTarget, speed);
	Focus(newTarget);


	SetPosition(newPos);
	SetOrientation(newTarget);
}

void CameraModule::SetFOV(const float fov, const float width, const float height)
{
	_fovH = fov;
	SetAspectRatio(width, height);

	_isDirty = true;
}

void CameraModule::SetAspectRatio(float width, float height)
{
	_aspectRatio = (float)width / (float)height;

	// Recompute vertical FOV based on the new aspect ratio
	_fovV = 2.0f * atan(tan(_fovH * 0.5f) / _aspectRatio);

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

void CameraModule::Resize(float width, float height)
{
	SetAspectRatio(width, height);
}

void CameraModule::CalculateProjectionMatrix()
{
	_proj = Matrix::CreatePerspectiveFieldOfView(_fovV, _aspectRatio, _nearPlane, _farPlane);
}

void CameraModule::CalculateViewMatrix() {
		
	_view = Matrix::CreateLookAt(_eye, _target, _up);
}
