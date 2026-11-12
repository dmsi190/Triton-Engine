// graphics.hpp

#pragma once

#include <optional>
#include <unordered_map>
#include "object.hpp"
#include "input_layout.hpp"
#include "render_target.hpp"
#include "render_pass_geometry.hpp"
#include "render_pass_processing.hpp"
#include "render_shading_model_enum.hpp"
#include "math.hpp"
#include "subsystem.hpp"
#include "types.hpp"

namespace triton
{
    class XVertexArrayPool;
    class XRenderTargetPool;
    class XRenderPassPool;

	class CGraphics : public CSubsystem
	{
        TRITON_CLASS_NAME(CGraphics)

        SHandle                             _inputLayoutStatic;
        SHandle                             _inputLayoutSkinned;
        SHandle                             _inputLayoutProcessing;
        SHandle                             _opaqueRigidPhongShader;
        SHandle                             _opaqueSkinnedPhongShader;
        SHandle                             _opaqueRigidPBRShader;
        SHandle                             _opaqueSkinnedPBRShader;
        SHandle                             _transparentShader;
        SHandle                             _textShader;
        SHandle                             _compositeTransparentShader;
        SHandle                             _compositeFinalShader;
        SHandle                             _opaqueRenderTarget;
        SHandle                             _transparentRenderTarget;
        SHandle                             _camera;
        SHandle                             _renderDomain;
        SHandle                             _opaqueRigid;
        SHandle                             _opaqueSkinned;
        SHandle                             _transparent;
        //SHandle _text;
        SHandle                             _compositeTransparent;
        SHandle                             _compositeFinal;

	public:
        explicit CGraphics(cContext* context);
        ~CGraphics() override;

        void ExecutePasses();

        void ResizeRenderTargets(const cVector2& size);
        
        void LoadShaderFiles(const std::string& vertexFuncPath, const std::string& fragmentFuncPath, std::string& vertexFunc, std::string& fragmentFunc);
        
        void SetShadingModel(EShadingModel shadingModel);

        inline const SHandle& GetCamera() const { return _camera; }

        const SHandle& GetOpaqueRigidRenderPass() const { return _opaqueRigid; }

        const SHandle& GetOpaqueSkinnedRenderPass() const { return _opaqueSkinned; }

        inline const SHandle& GetOpaqueRigidPBRShader() const { return _opaqueRigidPBRShader; }

        inline const SHandle& GetOpaqueSkinnedPBRShader() const { return _opaqueSkinnedPBRShader; }

        inline const SHandle& GetOpaqueRigidPhongShader() const { return _opaqueRigidPhongShader; }

        inline const SHandle& GetOpaqueSkinnedPhongShader() const { return _opaqueSkinnedPhongShader; }

    private:
        void CreateInputLayouts();

        void CreateShaders();

        CGPUTextureResource CreateRenderTargets();

        void CreateCameraAndRenderDomain();

        void CreateRenderPasses();

        void FinalizeSwapchain(const CGPUTextureResource& presentTexture);

        void DestroyInputLayouts();

        void DestroyShaders();

        void DestroyRenderTargets();

        void DestroyCameraAndRenderDomain();

        void DestroyRenderPasses();

        void ReleaseSwapchainResources();

        void BindBuffers();

        void UnbindBuffers();

        void ExecuteBuiltinPasses();
	};
}