#pragma once
#include "Module.h"
#include "InputModule.h"

class CameraModule: public Module
{
public:
	CameraModule() = default;
	~CameraModule();
	bool init();
	bool postInit();
	void update();

	void SetFOV(const float fov);
	void AspectRatio(const float aspectRatio);
	void SetPlaneDistances(const float near, const float far);
	void SetPosition(const Vector3& position);
	void SetOrientation(const Vector3& orientation);
	void LookAt(const Vector3& lookAt);
	void Resize();

	void CalculateProjectionMatrix();
	void CalculateViewMatrix();

	Matrix& GetProjectionMatrix() { return _proj; }
	Matrix& GetViewMatrix() { return _view; }
private:
	Matrix _view = Matrix::Identity;
	Matrix _proj = Matrix::Identity;

	Vector3 _eye = Vector3(0.0f, 10.f,10.f);
	Vector3 _target = Vector3::Zero;
	Vector3 _up = Vector3::Up;

	Vector3 _forward = Vector3::Forward;
	Vector3 _right = Vector3::Right;

	Quaternion _rotation = Quaternion::Identity;
	float _sensitivity = 0.004f;

	bool _isDirty = false;

	float _fov = XM_PIDIV4;
	float _aspectRatio = 0;
	float _nearPlane = 1.0f;
	float _farPlane = 1000.f;

	InputModule* _inputModule = nullptr;
};

