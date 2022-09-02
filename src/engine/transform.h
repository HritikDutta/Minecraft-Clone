#pragma once

#include "core/types.h"
#include "math/math.h"

struct Transform
{
public:
    Transform()
    :   _position(), _rotation(), _scale(1)
    ,   _dirty(true), _mat()
    ,   _parent(nullptr)
    {
    }

    Transform(Vector3 position)
    :   _position(position), _rotation(), _scale(1)
    ,   _dirty(true), _mat()
    ,   _parent(nullptr)
    {
    }

    Transform(Vector3 position, Vector3 rotation)
    :   _position(position), _rotation(rotation), _scale(1)
    ,   _dirty(true), _mat()
    ,   _parent(nullptr)
    {
    }

    Transform(Vector3 position, Vector3 rotation, Vector3 scale)
    :   _position(position), _rotation(rotation), _scale(scale)
    ,   _dirty(true), _mat()
    ,   _parent(nullptr)
    {
    }

    Transform(const Transform& other) = default;

    Vector3 position() const { return _position; }
    Vector3 rotation() const { return _rotation; }
    Vector3 scale()    const { return _scale; }

    Matrix4 matrix()
    {
        if (_dirty)
        {
            Matrix4 rotation = Matrix4::Rotation(Vector3::forward, _rotation.z)
                             * Matrix4::Rotation(Vector3::up, _rotation.y)
                             * Matrix4::Rotation(Vector3::right, _rotation.x);

            _mat = Matrix4::Translation(_position) * rotation * Matrix4::Scaling(_scale);
            _dirty = false;
        }
        
        return _mat;
    }

    Matrix4 worldMatrix()
    {
        if (!_parent)
            return matrix();

        Matrix4 p = _parent->worldMatrix();
        return p * matrix();
    }

    void SetPosition(Vector3 position)
    {
        _position = position;
        _dirty = true;
    }

    void SetRotation(Vector3 rotation)
    {
        _rotation = rotation;
        _dirty = true;
    }

    void SetScale(Vector3 scale)
    {
        _scale = scale;
        _dirty = true;
    }

    void SetParent(Transform* parent)
    {
        _parent = parent;
        // TODO: Recalculate transform according to parent's transform if it's not null
    }

private:
    Vector3 _position;
    Vector3 _rotation;
    Vector3 _scale;

    bool _dirty;
    Matrix4 _mat;

    Transform* _parent;
};