// graphics.hpp

#pragma once

#include "subsystem.hpp"
#include "types.hpp"

namespace triton
{
    class XShader;
    class XRenderTarget;
    class XCamera;
    class XRenderDomain;
    class XRenderPassCulling;
    class XRenderPassGeometry;

	class CGraphics : public CSubsystem
	{
        TRITON_CLASS_NAME(CGraphics)

        XShader*                    _opaque_rigid_phong_shader;
        XShader*                    _opaque_skinned_phong_shader;
        XShader*                    _opaque_rigid_pbr_shader;
        XShader*                    _opaque_skinned_pbr_shader;
        XRenderTarget*              _opaque_render_target;
        XCamera*                    _camera;
        XRenderDomain*              _render_domain;
        XRenderPassCulling*         _culling_pass;
        XRenderPassGeometry*        _opaque_rigid_pass;
        XRenderPassGeometry*        _opaque_skinned_pass;

	public:
        explicit CGraphics(cContext* context);

        ~CGraphics() override;

        void ExecutePasses();
        
        inline XCamera* GetCamera() const
        {
            return _camera;
        }

        inline XRenderPassCulling* GetCullingPass() const
        {
            return _culling_pass;
        }

        inline XRenderPassGeometry* GetOpaqueRigidPass() const
        {
            return _opaque_rigid_pass;
        }

        inline XRenderPassGeometry* GetOpaqueSkinnedPass() const
        {
            return _opaque_skinned_pass;
        }

    private:
        void CreateShaders();

        const SGPUTextureResource& CreateRenderTargets();

        void CreateCameraAndRenderDomain();

        void CreateRenderPasses();

        void DestroyShaders();

        void DestroyRenderTargets();

        void DestroyCameraAndRenderDomain();

        void DestroyRenderPasses();

        void ReleaseSwapchainResources();

        void ExecuteBuiltinPasses();
	};
}