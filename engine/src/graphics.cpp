// graphics.cpp

#include "graphics.hpp"
#include "context.hpp"
#include "thread_guard.hpp"
#include "graphics_backend.hpp"
#include "filesystem_manager.hpp"
#include "geometry_storage.hpp"
#include "texture.hpp"
#include "window.hpp"
#include "shader_stage_bit_enum.hpp"
#include "creator.hpp"
#include "application.hpp"
#include "graphics_buffers.hpp"
#include "render_command_proxy.hpp"
#include "texture_format.hpp"
#include "render_target.hpp"
#include "shader_texture_binding.hpp"
#include "gpu_buffer.hpp"
#include "render_pass_culling.hpp"
#include "render_pass_geometry.hpp"
#include "world_render_domain.hpp"

using namespace types;

void triton::XGraphics::CreateShaders()
{
    /*CFileSystem* fs = _context->GetSubsystem<CFileSystem>();

    const std::string pbrShaderPath =
        "C:/My/My_Projects_Programming/TritonEngine/runtime/data/shaders/builtin/brdf.shader";
    const std::string opaqueStaticVertexShaderPath =
        "C:/My/My_Projects_Programming/TritonEngine/runtime/data/shaders/builtin/opaque_vertex_static.shader";
    const std::string opaqueSkinnedVertexShaderPath =
        "C:/My/My_Projects_Programming/TritonEngine/runtime/data/shaders/builtin/opaque_vertex_skinned.shader";
    const std::string opaqueFragmentPBRShaderPath =
        "C:/My/My_Projects_Programming/TritonEngine/runtime/data/shaders/builtin/opaque_fragment_pbr.shader";
    const std::string opaqueFragmentPhongShaderPath =
        "C:/My/My_Projects_Programming/TritonEngine/runtime/data/shaders/builtin/opaque_fragment_phong.shader";
    const std::string transparentVertexShaderPath =
        "C:/My/My_Projects_Programming/TritonEngine/runtime/data/shaders/builtin/transparent_vertex.shader";
    const std::string transparentFragmentShaderPath =
        "C:/My/My_Projects_Programming/TritonEngine/runtime/data/shaders/builtin/transparent_fragment.shader";
    const std::string textVertexShaderPath =
        "C:/My/My_Projects_Programming/TritonEngine/runtime/data/shaders/builtin/text_vertex.shader";
    const std::string textFragmentShaderPath =
        "C:/My/My_Projects_Programming/TritonEngine/runtime/data/shaders/builtin/text_fragment.shader";
    const std::string compositeTransparentVertexShaderPath =
        "C:/My/My_Projects_Programming/TritonEngine/runtime/data/shaders/builtin/composite_transparent_vertex.shader";
    const std::string compositeTransparentFragmentShaderPath =
        "C:/My/My_Projects_Programming/TritonEngine/runtime/data/shaders/builtin/composite_transparent_fragment.shader";
    const std::string compositeFinalVertexShaderPath =
        "C:/My/My_Projects_Programming/TritonEngine/runtime/data/shaders/builtin/composite_final_vertex.shader";
    const std::string compositeFinalFragmentShaderPath =
        "C:/My/My_Projects_Programming/TritonEngine/runtime/data/shaders/builtin/composite_final_fragment.shader";

    std::string emptyStr = "";
    std::vector<SShaderDefine> emptyShaderDefineVec = {};
    std::vector<const char*> emptyConstCharVec = {};

    const std::vector<const char*> opaqueShaderFragmentIncludePaths = { pbrShaderPath.c_str() };

    auto opaqueStaticVertexStr = fs->TextFileToString(opaqueStaticVertexShaderPath);
    auto opaqueStaticPBRFragmentStr = fs->TextFileToString(opaqueFragmentPBRShaderPath);
    _opaqueRigidPBRShader = *_context->GetPool<CShaderPool>()->Create(
        opaqueStaticVertexStr,
        opaqueStaticPBRFragmentStr,
        emptyStr,
        emptyStr,
        emptyShaderDefineVec,
        emptyConstCharVec,
        opaqueShaderFragmentIncludePaths
    );

    auto opaqueSkinnedVertexStr = fs->TextFileToString(opaqueSkinnedVertexShaderPath);
    auto opaqueSkinnedPBRFragmentStr = fs->TextFileToString(opaqueFragmentPBRShaderPath);
    _opaqueSkinnedPBRShader = *_context->GetPool<CShaderPool>()->Create(
        opaqueSkinnedVertexStr,
        opaqueSkinnedPBRFragmentStr,
        emptyStr,
        emptyStr,
        emptyShaderDefineVec,
        emptyConstCharVec,
        opaqueShaderFragmentIncludePaths
    );

    auto opaqueStaticPhongFragmentStr = fs->TextFileToString(opaqueFragmentPhongShaderPath);
    _opaqueRigidPhongShader = *_context->GetPool<CShaderPool>()->Create(
        opaqueStaticVertexStr,
        opaqueStaticPhongFragmentStr,
        emptyStr,
        emptyStr,
        emptyShaderDefineVec,
        emptyConstCharVec,
        opaqueShaderFragmentIncludePaths
    );

    auto opaqueSkinnedPhongFragmentStr = fs->TextFileToString(opaqueFragmentPhongShaderPath);
    _opaqueSkinnedPhongShader = *_context->GetPool<CShaderPool>()->Create(
        opaqueSkinnedVertexStr,
        opaqueSkinnedPhongFragmentStr,
        emptyStr,
        emptyStr,
        emptyShaderDefineVec,
        emptyConstCharVec,
        opaqueShaderFragmentIncludePaths
    );

    auto transparentVertexStr = fs->TextFileToString(transparentVertexShaderPath);
    auto transparentFragmentStr = fs->TextFileToString(transparentFragmentShaderPath);
    _transparentShader = *_context->GetPool<CShaderPool>()->Create(
        transparentVertexStr,
        transparentFragmentStr,
        emptyStr,
        emptyStr,
        emptyShaderDefineVec,
        emptyConstCharVec,
        emptyConstCharVec
    );

    auto textVertexStr = fs->TextFileToString(textVertexShaderPath);
    auto textFragmentStr = fs->TextFileToString(textFragmentShaderPath);
    _textShader = *_context->GetPool<CShaderPool>()->Create(
        textVertexStr,
        textFragmentStr,
        emptyStr,
        emptyStr,
        emptyShaderDefineVec,
        emptyConstCharVec,
        emptyConstCharVec
    );

    auto compositeTransparentVertexStr = fs->TextFileToString(compositeTransparentVertexShaderPath);
    auto compositeTransparentFragmentStr = fs->TextFileToString(compositeTransparentFragmentShaderPath);
    _compositeTransparentShader = *_context->GetPool<CShaderPool>()->Create(
        compositeTransparentVertexStr,
        compositeTransparentFragmentStr,
        emptyStr,
        emptyStr,
        emptyShaderDefineVec,
        emptyConstCharVec,
        emptyConstCharVec
    );

    auto compositeFinalVertexStr = fs->TextFileToString(compositeFinalVertexShaderPath);
    auto compositeFinalFragmentStr = fs->TextFileToString(compositeFinalFragmentShaderPath);
    _compositeFinalShader = *_context->GetPool<CShaderPool>()->Create(
        compositeFinalVertexStr,
        compositeFinalFragmentStr,
        emptyStr,
        emptyStr,
        emptyShaderDefineVec,
        emptyConstCharVec,
        emptyConstCharVec
    );*/

    SShaderBytecodeFiles opaque_rigid_pbr_bytecode_files;
    opaque_rigid_pbr_bytecode_files.vertexFilePath =
        "C:/My/My_Projects_Programming/TritonEngine/runtime/data/shaders/builtin/passthrough.vert.spv";
    opaque_rigid_pbr_bytecode_files.pixelFilePath =
        "C:/My/My_Projects_Programming/TritonEngine/runtime/data/shaders/builtin/passthrough.frag.spv";

    _opaque_rigid_pbr_shader = CCreator::Create<XShader>(
        _context,
        (EShaderStageBit)((dword)EShaderStageBit::Vertex | (dword)EShaderStageBit::Pixel),
        opaque_rigid_pbr_bytecode_files
    );
}

const triton::SGPUTextureResource& triton::XGraphics::CreateRenderTargets()
{
    /*cVector2 windowSize = _context->GetSubsystem<CEngine>()->GetApplication()->GetWindow()->GetSize();
    _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_TEXTURE,
        windowSize.GetX(),
        windowSize.GetY(),
        0,
        (cpuword)ETextureDimension::Texture2D,
        (cpuword)ETextureFormat::RGBA8,
        (cpuword)nullptr,
        0
    ));
    CGPUTextureResource color = _context->GetSubsystem<CEngine>()->GetSynchronization()->WaitForRenderCommandResult<CGPUTextureResource>();
    _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_TEXTURE,
        windowSize.GetX(),
        windowSize.GetY(),
        0,
        (cpuword)ETextureDimension::Texture2D,
        (cpuword)ETextureFormat::RGBA16F,
        (cpuword)nullptr,
        0
    ));
    CGPUTextureResource accumulation = _context->GetSubsystem<CEngine>()->GetSynchronization()->WaitForRenderCommandResult<CGPUTextureResource>();
    _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_TEXTURE,
        windowSize.GetX(),
        windowSize.GetY(),
        0,
        (cpuword)ETextureDimension::Texture2D,
        (cpuword)ETextureFormat::R8F,
        (cpuword)nullptr,
        0
    ));
    CGPUTextureResource revealage = _context->GetSubsystem<CEngine>()->GetSynchronization()->WaitForRenderCommandResult<CGPUTextureResource>();
    _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_TEXTURE,
        windowSize.GetX(),
        windowSize.GetY(),
        0,
        (cpuword)ETextureDimension::Texture2D,
        (cpuword)ETextureFormat::DepthStencil,
        (cpuword)nullptr,
        0
    ));
    CGPUTextureResource depth = _context->GetSubsystem<CEngine>()->GetSynchronization()->WaitForRenderCommandResult<CGPUTextureResource>();

    CRenderTargetPool* rtPool = _context->GetPool<CRenderTargetPool>();

    std::vector<CGPUTextureResource> opaqueColorAttachments = { color };
    _opaqueRenderTarget = *rtPool->Create(opaqueColorAttachments, depth);

    std::vector<CGPUTextureResource> transparentColorAttachments = { accumulation, revealage };
    _transparentRenderTarget = *rtPool->Create(transparentColorAttachments, depth);*/

    const cVector2& application_window_size =
        _context->GetSubsystem<XEngine>()->GetApplication()->GetWindow()->GetSize();

    XTexture* color_texture = CCreator::Create<XTexture>(
        _context,
        True,
        ETextureFormat::RGBA8,
        (ETextureUsageBit)((dword)ETextureUsageBit::Sampled | (dword)ETextureUsageBit::ColorAttachment),
        ETextureDimension::Texture2D,
        cVector3(application_window_size.GetX(), application_window_size.GetY(), 1.0f),
        nullptr,
        0
    );

    XTexture* depth_texture = CCreator::Create<XTexture>(
        _context,
        True,
        ETextureFormat::DepthStencil,
        ETextureUsageBit::DepthStencilAttachment,
        ETextureDimension::Texture2D,
        cVector3(application_window_size.GetX(), application_window_size.GetY(), 1.0f),
        nullptr,
        0
    );

    _opaque_render_target = CCreator::Create<XRenderTarget>(
        _context,
        std::vector<XTexture*>({ color_texture }),
        depth_texture
    );

    const SGPUTextureResource& swapchain_present_texture = color_texture->GetGPUResource();

    return swapchain_present_texture;
}

void triton::XGraphics::CreateCameraAndRenderDomain()
{
    _camera = CCreator::Create<XCamera>(_context, 65.0f, 0.01f, 1000.0f);

    _render_domain = CCreator::Create<XRenderDomain>(_context, _camera);
}

void triton::XGraphics::CreateRenderPasses()
{
    /*cVector2 windowSize = _context->GetSubsystem<CEngine>()->GetApplication()->GetWindow()->GetSize();
    CFileSystem* fs = _context->GetSubsystem<CFileSystem>();

    SViewport viewport;
    viewport.rect = cVector4(0.0f, 0.0f, windowSize.GetX(), windowSize.GetY());

    CRenderPassGeometryPool* geometryRPPool = _context->GetPool<CRenderPassGeometryPool>();
    CRenderPassProcessingPool* processingRPPool = _context->GetPool<CRenderPassProcessingPool>();

    std::string emptyStr = "";
    std::vector<SShaderDefine> emptyShaderDefineVec = {};
    std::vector<const char*> emptyConstCharVec = {};

    SBlendState opaqueBlendState = {};
    opaqueBlendState.factorCount = 1;
    opaqueBlendState.srcFactors[0] = EBlendFactor::ONE;
    opaqueBlendState.dstFactors[0] = EBlendFactor::ZERO;

    // Opaque static render pass
    _opaqueStatic = *geometryRPPool->Create();
    XRenderPassGeometry& opaqueStaticData = *geometryRPPool->Get(_opaqueStatic);
    opaqueStaticData.SetClearState(SClearState(cVector4(0.055f, 0.075f, 0.090f, 1.0f) * 2.0f, 1.0f));
    opaqueStaticData.SetInputLayout(_inputLayoutStatic);
    opaqueStaticData.SetShader(_opaqueRigidPBRShader);
    opaqueStaticData.SetViewport(viewport);
    opaqueStaticData.SetDepthState(SDepthState(K_TRUE, K_TRUE));
    opaqueStaticData.SetBlendState(opaqueBlendState);
    opaqueStaticData.SetRenderTarget(_opaqueRenderTarget);

    // Opaque skinned render pass
    _opaqueSkinned = *geometryRPPool->Create();
    XRenderPassGeometry& opaqueSkinnedData = *geometryRPPool->Get(_opaqueSkinned);
    opaqueSkinnedData.SetClearState(std::nullopt);
    opaqueSkinnedData.SetInputLayout(_inputLayoutSkinned);
    opaqueSkinnedData.SetShader(_opaqueSkinnedPBRShader);
    opaqueSkinnedData.SetViewport(viewport);
    opaqueSkinnedData.SetDepthState(SDepthState(K_TRUE, K_TRUE));
    opaqueSkinnedData.SetBlendState(opaqueBlendState);
    opaqueSkinnedData.SetRenderTarget(_opaqueRenderTarget);

    // Transparent render pass
    SBlendState transparentBlendState = {};
    transparentBlendState.factorCount = 2;
    transparentBlendState.srcFactors[0] = EBlendFactor::ONE;
    transparentBlendState.dstFactors[0] = EBlendFactor::ONE;
    transparentBlendState.srcFactors[1] = EBlendFactor::ZERO;
    transparentBlendState.dstFactors[1] = EBlendFactor::INV_SRC_COLOR;

    _transparent = *geometryRPPool->Create();
    XRenderPassGeometry& transparentData = *geometryRPPool->Get(_transparent);
    transparentData.SetInputLayout(_inputLayoutStatic);
    transparentData.SetShader(_transparentShader);
    transparentData.SetViewport(viewport);
    transparentData.SetDepthState(SDepthState(K_TRUE, K_FALSE));
    transparentData.SetBlendState(transparentBlendState);
    transparentData.SetRenderTarget(_transparentRenderTarget);

    // Text render pass
    auto textVertexStr = fs->TextFileToString(textVertexShaderPath);
    auto textFragmentStr = fs->TextFileToString(textFragmentShaderPath);
    XShader::THandle textShader = *_context->GetPool<CShaderPool>()->Create(
        textVertexStr,
        textFragmentStr,
        emptyStr,
        emptyStr,
        emptyShaderDefineVec,
        emptyConstCharVec,
        emptyConstCharVec
    );

    _text = *rpPool->Create();
    XRenderPass& textData = *rpPool->Get(_text);
    textData.SetDispatch(ERenderPassDispatch::Text);
    textData.SetBatchFormat(EVertexBufferFormat::Unknown);
    textData.SetShader(textShader);
    textData.SetViewport(viewport);
    textData.SetDepthState(SDepthState(K_FALSE, K_FALSE));
    textData.SetRenderTarget(_opaqueRenderTarget);

    // Composite transparent render pass
    SBlendState compositeTransparentBlendState = {};
    compositeTransparentBlendState.factorCount = 1;
    compositeTransparentBlendState.srcFactors[0] = EBlendFactor::SRC_ALPHA;
    compositeTransparentBlendState.dstFactors[0] = EBlendFactor::INV_SRC_ALPHA;

    XRenderTarget& transparentRTData = *_context->GetPool<CRenderTargetPool>()->Get(_transparentRenderTarget);
    const std::vector<CGPUBufferResource> compositeTransparentInputBuffers = {};
    const std::vector<SShaderTextureBinding> compositeTransparentInputTextures = {
        SShaderTextureBinding("AccumulationTexture", transparentRTData.GetGPUResource().GetColorAttachments()[0]),
        SShaderTextureBinding("RevealageTexture", transparentRTData.GetGPUResource().GetColorAttachments()[1])
    };
    _compositeTransparent = *processingRPPool->Create(
        _inputLayoutProcessing,
        compositeTransparentInputBuffers,
        compositeTransparentInputTextures,
        viewport,
        compositeTransparentBlendState,
        SDepthState(False, False),
        _opaqueRenderTarget,
        _compositeTransparentShader
    );

    // Composite final render pass
    SBlendState compositeFinalBlendState = {};
    compositeFinalBlendState.factorCount = 1;
    compositeFinalBlendState.srcFactors[0] = EBlendFactor::ONE;
    compositeFinalBlendState.dstFactors[0] = EBlendFactor::ZERO;

    XRenderTarget& opaqueRTData = *_context->GetPool<CRenderTargetPool>()->Get(_opaqueRenderTarget);
    const std::vector<CGPUBufferResource> compositeFinalInputBuffers = {};
    const std::vector<SShaderTextureBinding> compositeFinalInputTextures = {
        SShaderTextureBinding("colorTexture", opaqueRTData.GetGPUResource().GetColorAttachments()[0])
    };
    _compositeFinal = *processingRPPool->Create(
        _inputLayoutProcessing,
        compositeFinalInputBuffers,
        compositeFinalInputTextures,
        viewport,
        compositeFinalBlendState,
        SDepthState(False, False),
        XRenderTarget::THandle(),
        _compositeFinalShader
    );*/

    {
        _culling_pass = CCreator::Create<XRenderPassCulling>(_context);
    }

    {
        SViewport viewport;
        viewport.rect = cVector4(
            0,
            0,
            _opaque_render_target->GetColorAttachmentSize(0).GetX(),
            _opaque_render_target->GetColorAttachmentSize(0).GetY()
        );

        const std::vector<ERenderResourceUsage> src_render_target_attachments_usage = {
            ERenderResourceUsage::Unknown, ERenderResourceUsage::Unknown
        };
        const std::vector<ERenderResourceUsage> dst_render_target_attachments_usage = {
            ERenderResourceUsage::PixelShaderRead, ERenderResourceUsage::DepthAttachment
        };

        const std::vector<SGPUBufferResource> input_buffers = {
            CGraphicsBuffers::GetStaticInstanceBuffer()->GetGPUResource()
        };

        const std::vector<SShaderTextureBinding> input_textures;

        _opaque_rigid_pass = CCreator::Create<XRenderPassGeometry>(
            _context,
            _render_domain,
            _opaque_render_target,
            True,
            cVector4(0.45f, 0.45f, 0.45f, 1.0f),
            1.0f,
            src_render_target_attachments_usage,
            dst_render_target_attachments_usage,
            _opaque_rigid_pbr_shader,
            ERenderPrimitiveTopology::TriangleList,
            viewport,
            input_buffers,
            input_textures
        );
    }
}

void triton::XGraphics::DestroyShaders()
{
    CCreator::Destroy<XShader>(_opaque_rigid_pbr_shader);
}

void triton::XGraphics::DestroyRenderTargets()
{
    for (auto attachment : _opaque_render_target->GetColorAttachments())
        CCreator::Destroy<XTexture>(attachment);

    CCreator::Destroy<XTexture>(_opaque_render_target->GetDepthAttachment());

    CCreator::Destroy<XRenderTarget>(_opaque_render_target);
}

void triton::XGraphics::DestroyCameraAndRenderDomain()
{
    CCreator::Destroy<XRenderDomain>(_render_domain);

    CCreator::Destroy<XCamera>(_camera);
}

void triton::XGraphics::DestroyRenderPasses()
{
    CCreator::Destroy<XRenderPassGeometry>(_opaque_rigid_pass);

    CCreator::Destroy<XRenderPassCulling>(_culling_pass);
}

void triton::XGraphics::ReleaseSwapchainResources()
{
    CRenderCommandProxy::ReleaseSwapchainResources();
}

void triton::XGraphics::ExecuteBuiltinPasses()
{
    _culling_pass->Render();
    _opaque_rigid_pass->Render();
}

triton::XGraphics::XGraphics(const SMemAlloc& mem_alloc, cContext* context) : XSubsystem(mem_alloc, context)
{
    // TODO Vulkan backend: uncomment and rewrite when Vulkan backend will be ready
    // ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
    // VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV

    CreateShaders();

    const SGPUTextureResource& swapchain_present_texture = CreateRenderTargets();

    CreateCameraAndRenderDomain();

    CreateRenderPasses();

    CRenderCommandProxy::FinalizeSwapchain(swapchain_present_texture);
}

triton::XGraphics::~XGraphics()
{
    // TODO Vulkan backend: uncomment and rewrite when Vulkan backend will be ready
    // ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
    // VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV

    ReleaseSwapchainResources();

    DestroyRenderPasses();

    DestroyCameraAndRenderDomain();

    DestroyRenderTargets();

    DestroyShaders();
}

void triton::XGraphics::ExecutePasses()
{
    // TODO: [Vulkan backend] This must be done using render command queue on main thread
    // ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
    // VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
    //BindBuffers();
    ExecuteBuiltinPasses();
    //UnbindBuffers();
}

// TODO: Implement new text drawing approach
/*void cGraphics::DrawTexts(const std::vector<cGameObject>& objects)
{
    for (auto& it : objects)
    {
        if (it.GetText() == nullptr)
            continue;

        const cText* text = it.GetText();
        const cFontFace* textFont = text->GetFont();
        const std::string& textString = text->GetText();
        const auto& alphabet = textFont->GetAlphabet();
        const cTexture* atlas = text->GetFont()->GetAtlas();

        _textInstancesByteSize = 0;
        _materialsMap->clear();

        const cTransform transform(&it);

        IApplication* app = _context->GetSubsystem<CEngine>()->GetApplication();
        const glm::vec2 windowSize = app->GetWindow()->GetSize();
        const glm::vec2 textPosition = glm::vec2((transform._position.x * 2.0f) - 1.0f, (transform._position.y * 2.0f) - 1.0f);
        const glm::vec2 textScale = glm::vec2(
            (1.0f / windowSize.x) * it.GetTransform()->_scale.x,
            (1.0f / windowSize.y) * it.GetTransform()->_scale.y
        );

        const usize charCount = textString.length();
        usize actualCharCount = 0;
        glm::vec2 offset = glm::vec2(0.0f);
        for (usize i = 0; i < charCount; i++)
        {
            const u8 glyphChar = textString[i];

            if (glyphChar == '\t')
            {
                offset.x += textFont->GetOffsetTab() * textScale.x;
                continue;
            }
            else if (glyphChar == '\n')
            {
                offset.x = 0.0f;
                offset.y -= textFont->GetOffsetNewline() * textScale.y;
                continue;
            }
            else if (glyphChar == ' ')
            {
                offset.x += textFont->GetOffsetSpace() * textScale.x;
                continue;
            }

            const auto alphabetEntry = alphabet.find(glyphChar);
            if (alphabetEntry == alphabet.end())
                continue;
            const sGlyph& glyph = alphabetEntry->second;

            sTextInstance t;
            t._info.x = textPosition.x + offset.x;
            t._info.y = textPosition.y + (offset.y - (float)((glyph._height - glyph._top) * textScale.y));
            t._info.z = (float)glyph._width * textScale.x;
            t._info.w = (float)glyph._height * textScale.y;
            t._atlasInfo.x = (float)glyph._atlasXOffset / (float)atlas->GetWidth();
            t._atlasInfo.y = (float)glyph._atlasYOffset / (float)atlas->GetHeight();
            t._atlasInfo.z = (float)glyph._width / (float)atlas->GetWidth();
            t._atlasInfo.w = (float)glyph._height / (float)atlas->GetHeight();

            offset.x += glyph._advanceX * textScale.x;

            memcpy((void*)((usize)_textInstances + _textInstancesByteSize), &t, sizeof(sTextInstance));
            _textInstancesByteSize += sizeof(sTextInstance);

            actualCharCount += 1;
        }

        const cMaterialInstance mi(0, it.GetMaterial());
        memcpy(_textMaterials, &mi, sizeof(cMaterialInstance));
        _textMaterialsByteSize += sizeof(cMaterialInstance);

        _gfx->WriteBuffer(_textInstanceBuffer, 0, _textInstancesByteSize, _textInstances);
        _gfx->WriteBuffer(_textMaterialBuffer, 0, _textMaterialsByteSize, _textMaterials);

        _gfx->BindRenderPass(_text);
        _gfx->BindTexture(_text->GetShader(), "FontAtlas", atlas, 0);
        _gfx->DrawQuads(actualCharCount);
        _gfx->UnbindRenderPass(_text);
    }
}*/