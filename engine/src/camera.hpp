// camera.hpp

#pragma once

#include "math.hpp"
#include "object.hpp"
#include "types.hpp"

namespace triton
{
    class XRenderPassGeometry;

    class XCamera : public iObject
    {
        TRITON_OBJECT(XCamera)

        types::f32 _fov = 0.0f;
        types::f32 _zNear = 0.0f;
        types::f32 _zFar = 0.0f;
        cVector3 _eulerAngles = cVector3(0.0f);
        cVector3 _worldPosition = cVector3(0.0f);
        cVector3 _worldDirection = cVector3(0.0f);
        cMatrix4 _viewMatrix = cMatrix4(1.0f);
        cMatrix4 _projectionMatrix = cMatrix4(1.0f);
        cMatrix4 _viewProjectionMatrix = cMatrix4(1.0f);
        cVector2 _previousCursorPosition = cVector2(0.0f);
        cVector2 _cursorPosition = cVector2(0.0f);
        glm::vec4 left_plane;
        glm::vec4 right_plane;
        glm::vec4 bottom_plane;
        glm::vec4 top_plane;
        glm::vec4 near_plane;
        glm::vec4 far_plane;

    public:
        explicit XCamera(
            const SMemAlloc& mem_alloc,
            cContext* context,
            types::f32 fov,
            types::f32 zNear,
            types::f32 zFar
        );

        ~XCamera() override = default;

        void Bind(XRenderPassGeometry* pass);

        void Update(
            const cVector2& mouseDelta,
            types::usize screenWidth,
            types::usize screenHeight,
            types::f32 mouseSensitivity
        );

        void AddEuler(CMath::EEulerAngle angle, types::f32 value);

        void Move(types::f32 value);

        void Strafe(types::f32 value);

        void Lift(types::f32 value);

        inline const cVector3& GetWorldPosition() const { return _worldPosition; }

        inline const cMatrix4& GetViewProjectionMatrix() const { return _viewProjectionMatrix; }

        inline void SetEulerAngles(const cVector3& eulerAngles) { _eulerAngles = eulerAngles; }

        inline void SetWorldPosition(const cVector3& worldPosition) { _worldPosition = worldPosition; }

        struct TGPULayout
        {
            glm::vec4 left_plane;
            glm::vec4 right_plane;
            glm::vec4 bottom_plane;
            glm::vec4 top_plane;
            glm::vec4 near_plane;
            glm::vec4 far_plane;
        };
    };
}