// graphics.hpp

#pragma once

#include "subsystem.hpp"
#include "gpu_texture_resource.hpp"
#include "types.hpp"

namespace triton
{
    class XShader;
    class XRenderTarget;
    class XCamera;
    class XRenderDomain;
    class XRenderPassCulling;
    class XRenderPassGeometry;

	class XGraphics : public XSubsystem
	{
        TRITON_OBJECT(XGraphics)

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
        explicit XGraphics(const SMemAlloc& mem_alloc, cContext* context);

        ~XGraphics() override;

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