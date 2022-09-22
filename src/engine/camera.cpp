#include "camera.h"

#include "core/types.h"
#include "core/input.h"
#include "math/math.h"

Camera Camera::Orthographic(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far)
{
    Camera c;
    c._projection = Matrix4::Orthographic(left, right, bottom, top, near, far);

    c._aspectRatio = (f32) (right - left) / (top - bottom);
    c._fov = 0.0f;
    c._near = near;
    c._far = far;
    
    return c;
}

Camera Camera::Perspective(f32 fov, f32 aspectRatio, f32 near, f32 far)
{
    Camera c;
    c._projection = Matrix4::Perspective(fov, aspectRatio, near, far);

    c._aspectRatio = aspectRatio;
    c._fov = fov;
    c._near = near;
    c._far = far;
    
    return c;

}

static bool HandleMouseInput(Camera& camera, f32 lookSpeed, f32 time, bool freeLook)
{
    if (!freeLook && !Input::GetMouseButton(MouseButton::RIGHT))
        return false;

    f32& pitch = camera.pitch();
    f32& yaw = camera.yaw();

    if (Input::DeltaMousePosition() == Vector2(0.0f))
        return false;

    yaw   += Input::DeltaMousePosition().x * lookSpeed * time;
    pitch -= Input::DeltaMousePosition().y * lookSpeed * time;

    pitch = Clamp(pitch, Math::DegToRad * -75.0f, Math::DegToRad * 75.0f);

    Vector3& forward = camera.forward();

    forward.x = Math::Cos(yaw) * Math::Cos(pitch);
    forward.y = sin(pitch);
    forward.z = sin(yaw) * Math::Cos(pitch);

    forward = forward.Normalized();

    camera.UpdateDirections();
    camera.UpdateViewMatrix();

    return true;
}

static bool HandleKeyboardInput(Camera& camera, f32 moveSpeed, f32 time)
{
    u8 moveFlags = 0;

    #define MOVE_FLAGS_FORWARD  (1 << 0)
    #define MOVE_FLAGS_BACKWARD (1 << 1)
    #define MOVE_FLAGS_LEFT     (1 << 2)
    #define MOVE_FLAGS_RIGHT    (1 << 3)
    #define MOVE_FLAGS_UP       (1 << 4)
    #define MOVE_FLAGS_DOWN     (1 << 5)
    
    if (Input::GetKey(Key::W))
        moveFlags |= MOVE_FLAGS_FORWARD;

    if (Input::GetKey(Key::S))
        moveFlags |= MOVE_FLAGS_BACKWARD;

    if (Input::GetKey(Key::A))
        moveFlags |= MOVE_FLAGS_LEFT;

    if (Input::GetKey(Key::D))
        moveFlags |= MOVE_FLAGS_RIGHT;

    if (Input::GetKey(Key::E))
        moveFlags |= MOVE_FLAGS_UP;

    if (Input::GetKey(Key::Q))
        moveFlags |= MOVE_FLAGS_DOWN;

    f32 forward  = 1.0f * (f32) ((moveFlags & MOVE_FLAGS_FORWARD)  != 0)
                 - 1.0f * (f32) ((moveFlags & MOVE_FLAGS_BACKWARD) != 0);

    f32 sideways = 1.0f * (f32) ((moveFlags & MOVE_FLAGS_RIGHT) != 0)
                 - 1.0f * (f32) ((moveFlags & MOVE_FLAGS_LEFT)  != 0);
                 
    f32 vertical = 1.0f * (f32) ((moveFlags & MOVE_FLAGS_UP)   != 0)
                 - 1.0f * (f32) ((moveFlags & MOVE_FLAGS_DOWN) != 0);

    if (moveFlags)
    {
        Vector3 direction = (camera.forward() * forward + camera.right() * sideways + camera.up() * vertical).Normalized();

        f32 distance = moveSpeed * time;
        camera.position() += distance * direction;
        camera.UpdateViewMatrix();
    }

    #undef MOVE_FLAGS_FORWARD
    #undef MOVE_FLAGS_BACKWARD
    #undef MOVE_FLAGS_LEFT
    #undef MOVE_FLAGS_RIGHT
    #undef MOVE_FLAGS_UP
    #undef MOVE_FLAGS_DOWN

    return (moveFlags != 0);
}

bool MoveCamera(Camera& camera, f32 lookSpeed, f32 moveSpeed, f32 time, bool freeLook)
{
    const bool res1 = HandleMouseInput(camera, lookSpeed, time, freeLook);
    const bool res2 = HandleKeyboardInput(camera, moveSpeed, time);
    return res1 || res2;
}