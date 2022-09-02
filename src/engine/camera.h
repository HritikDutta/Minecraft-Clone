#pragma once

#include "core/types.h"
#include "math/math.h"

class Camera
{
public:
    static Camera Orthographic(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far);
    static Camera Perspective(f32 fov, f32 aspectRatio, f32 near, f32 far);

    const Vector3& position() const { return _position; }
    Vector3& position() { return _position; }

    const Vector3& forward() const { return _forward; }
    Vector3& forward() { return _forward; }

    const Vector3& up() const { return _up; }
    Vector3& up() { return _up; }

    const Vector3& right() const { return _right; }

    const Frustum& viewFrustum()
    {
        return _viewFrustum;
    }

    void UpdateDirections() { _right = Cross(_forward, _up).Normalized(); }

    void UpdateViewFrustum()
    {
        Matrix4 M = (_projection * _view).Transpose();

        _viewFrustum.left.vector  = (M._vector[3] + M._vector[0]);
        _viewFrustum.right.vector = (M._vector[3] - M._vector[0]);

        _viewFrustum.bottom.vector = (M._vector[3] + M._vector[1]);
        _viewFrustum.top.vector    = (M._vector[3] - M._vector[1]);

        _viewFrustum.near.vector = (M._vector[3] + M._vector[2]);
        _viewFrustum.far.vector  = (M._vector[3] - M._vector[2]);
    }

    void UpdateYawAndPitch()
    {
        _pitch = asinf(_forward.y);
        _yaw = atanf(_forward.z / _forward.x);

         if (_forward.x < 0.0f)
            _yaw = Math::PI + _yaw;
    }

    const Matrix4& projection() const { return _projection; }

    void SetProjection(const Matrix4& projMatrix)
    {
        _projection = projMatrix;
        _matrixUpdated = true;
    }

    const Matrix4& viewProjection()
    {
        if (_matrixUpdated)
        {
            _viewProjection = _projection * _view;
            _matrixUpdated = false;
        }

        return _viewProjection;
    }

    void UpdateViewMatrix()
    {
        _view = Matrix4::LookAt(_position, _position + _forward, _up);
        _matrixUpdated = true;
    }

    const f32& pitch() const { return _pitch; }
    f32& pitch() { return _pitch; }

    const f32& yaw() const { return _yaw; }
    f32& yaw() { return _yaw; }

private:
    Matrix4 _projection;
    Matrix4 _view;
    Matrix4 _viewProjection;
    bool _matrixUpdated = false;

    Vector3 _position;
    Vector3 _forward;
    Vector3 _right;
    Vector3 _up;

    Frustum _viewFrustum;
    f32 _aspectRatio;
    f32 _fov;
    f32 _near, _far;

    f32 _yaw, _pitch;
};

void MoveCamera(Camera& camera, f32 lookSpeed, f32 moveSpeed, f32 time, bool freeLook = false);