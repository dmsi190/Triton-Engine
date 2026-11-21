// graphics_backend_ogl.cpp

#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
#include <SDL3/SDL.h>
#include "graphics_backend_ogl.hpp"
#include "context.hpp"
#include "filesystem_manager.hpp"
#include "application.hpp"
#include "log.hpp"

using namespace gl;
using namespace types;

namespace triton
{
    static void GLDebugCallback(
        GLenum source,
        GLenum type,
        GLuint id,
        GLenum severity,
        GLsizei length,
        const GLchar* message,
        const void* userParam
    )
    {
        Print(message);
    }

    std::string CleanShaderSource(const std::string& src)
    {
        std::string out;
        out.reserve(src.size());
        for (u8 c : src)
        {
            if (c == '\t' || c == '\n' || c == '\r' || (c >= 32 && c <= 126))
                out.push_back(c);
        }

        return out;
    }

    void DefineInShader(std::string& shaderVertexStr, std::string& shaderFragmentStr, const std::vector<SShaderDefine>& definePairs)
    {
        if (!definePairs.empty())
        {
            std::string defineStr = "";
            for (const auto& define : definePairs)
                defineStr += "#define " + define.name + " " + std::to_string(define.index) + "\n";

            shaderVertexStr = defineStr + shaderVertexStr;
            shaderFragmentStr = defineStr + shaderFragmentStr;
        }
    }
}

void triton::XGraphicsBackendOGL::CreateGraphicsContext(SWindowResource& window)
{
    window.renderContextInstance = (qword)SDL_GL_CreateContext((SDL_Window*)window.instance);
    if (!window.renderContextInstance)
    {
        Print("Error: Failed to create GL context");
        return;
    }

    glbinding::initialize(SDL_GL_GetProcAddress);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glDepthFunc(GL_LESS);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(GLDebugCallback, nullptr);
}

void triton::XGraphicsBackendOGL::MakeWindowGraphicsContextCurrent(const SWindowResource& window)
{
    SDL_GL_MakeCurrent((SDL_Window*)window.instance, (SDL_GLContext)window.renderContextInstance);
}

void triton::XGraphicsBackendOGL::SwapWindowBuffers(const SWindowResource& window)
{
    SDL_GL_SwapWindow((SDL_Window*)window.instance);
}

void triton::XGraphicsBackendOGL::ClearColor(const cVector4& color)
{
    glClearColor(color.GetX(), color.GetY(), color.GetZ(), color.GetW());
    glClear(GL_COLOR_BUFFER_BIT);
}

void triton::XGraphicsBackendOGL::ClearDepth(const f32 depth)
{
    glClearDepth(depth);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void triton::XGraphicsBackendOGL::ClearFramebufferColor(usize bufferIndex, const cVector4& color)
{
    GLfloat buff[] = { color.GetX(), color.GetY(), color.GetZ(), color.GetW() };
    glClearBufferfv(GL_COLOR, bufferIndex, &buff[0]);
}

void triton::XGraphicsBackendOGL::ClearFramebufferDepth(f32 depth)
{
    glClearDepth(depth);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void triton::XGraphicsBackendOGL::Draw(
    usize indexCount,
    usize vertexElementOffset,
    usize indexElementOffset,
    usize instanceCount
)
{
    glDrawElementsInstancedBaseVertex(
        GL_TRIANGLES,
        indexCount,
        GL_UNSIGNED_INT,
        (const void*)(indexElementOffset * sizeof(u32)),
        instanceCount,
        vertexElementOffset
    );
}

void triton::XGraphicsBackendOGL::DrawQuad()
{
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void triton::XGraphicsBackendOGL::DrawQuads(usize count)
{
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, count);
}

triton::SGPUShaderResource triton::XGraphicsBackendOGL::CreateShader(
    const char* vertexStr,
    const char* fragmentStr,
    const char* vertexCustomFuncStr,
    const char* fragmentCustomFuncStr,
    types::usize defineCount,
    const SShaderDefine* defines,
    types::usize vertexIncludePathCount,
    const char** vertexIncludePaths,
    types::usize fragmentIncludePathCount,
    const char** fragmentIncludePaths
)
{
    std::string vertexCppStr = vertexStr;
    std::string fragmentCppStr = fragmentStr;

    const std::string appendStr = "#version 430\n\n";
    const std::string vertexFuncDefinition = "void Vertex_Func(in vec3 _positionLocal, in vec2 _texcoord, in vec3 _normal, in int _instanceID, in Instance _instance, in Material material, in float _use2D, out vec4 _glPosition){}";
    const std::string vertexFuncPassthroughCall = "Vertex_Passthrough(InPositionLocal, instance, instance.Use2D, gl_Position);";
    const std::string fragmentFuncDefinition = "void Fragment_Func(in vec2 _texcoord, in vec4 _textureColor, in vec4 _materialDiffuseColor, out vec4 _fragColor){}";
    const std::string fragmentFuncPassthroughCall = "Fragment_Passthrough(textureColor, DiffuseColor, fragColor);";

    if (!vertexCustomFuncStr && !fragmentCustomFuncStr)
    {
        const usize vertexFuncDefinitionPos = vertexCppStr.find(vertexFuncDefinition);
        if (vertexFuncDefinitionPos != std::string::npos)
            vertexCppStr.replace(vertexFuncDefinitionPos, vertexFuncDefinition.length(), vertexCustomFuncStr);
        const usize vertexFuncPasstroughCallPos = vertexCppStr.find(vertexFuncPassthroughCall);
        if (vertexFuncPasstroughCallPos != std::string::npos)
            vertexCppStr.replace(vertexFuncPasstroughCallPos, vertexFuncPassthroughCall.length(), "");
        const usize fragmentFuncDefinitionPos = fragmentCppStr.find(fragmentFuncDefinition);
        if (fragmentFuncDefinitionPos != std::string::npos)
            fragmentCppStr.replace(fragmentFuncDefinitionPos, fragmentFuncDefinition.length(), fragmentCustomFuncStr);
        const usize fragmentFuncPassthroughPos = fragmentCppStr.find(fragmentFuncPassthroughCall);
        if (fragmentFuncPassthroughPos != std::string::npos)
            fragmentCppStr.replace(fragmentFuncPassthroughPos, fragmentFuncPassthroughCall.length(), "");
    }

    std::vector<SShaderDefine> definesVec = {};
    for (usize i = 0; i < defineCount; i++)
        definesVec.push_back(defines[i]);
    DefineInShader(vertexCppStr, fragmentCppStr, definesVec);

    std::string vertexCppStrInclude = "";
    std::string fragmentCppStrInclude = "";
    XFileSystem* fs = _context->GetSubsystem<XFileSystem>();
    /*if (vertexIncludePathCount > 0)
    {
        usize vertexIncludeBufferByteCount = 0;
        for (usize i = 0; i < vertexIncludePathCount; i++)
            vertexIncludeBufferByteCount += fs->TellFileByteSize(vertexIncludePaths[i]);
        u8* vertexIncludeBuffer = (u8*)ma->Allocate(vertexIncludeBufferByteCount, 64);
        usize vertexIncludeBufferPtr = 0;
        // TODO: rewrite using BinaryFileToArray()
        //for (usize i = 0; i < vertexIncludePathCount; i++)
        //    vertexIncludeBufferPtr += fs->BinFileToArray(vertexIncludePaths[i], &vertexIncludeBuffer[0], vertexIncludeBufferPtr, vertexIncludeBufferByteCount);
        vertexCppStrInclude = std::string((const char*)&vertexIncludeBuffer[0], vertexIncludeBufferByteCount);
        ma->Deallocate(vertexIncludeBuffer);
    }
    if (fragmentIncludePathCount > 0)
    {
        usize fragmentIncludeBufferByteCount = 0;
        for (usize i = 0; i < fragmentIncludePathCount; i++)
            fragmentIncludeBufferByteCount += fs->TellFileByteSize(fragmentIncludePaths[i]);
        u8* fragmentIncludeBuffer = (u8*)ma->Allocate(fragmentIncludeBufferByteCount, 64);
        usize fragmentIncludeBufferPtr = 0;
        // TODO: rewrite using BinaryFileToArray()
        //for (usize i = 0; i < fragmentIncludePathCount; i++)
        //    fragmentIncludeBufferPtr += fs->BinFileToArray(fragmentIncludePaths[i], &fragmentIncludeBuffer[0], fragmentIncludeBufferPtr, fragmentIncludeBufferByteCount);
        fragmentCppStrInclude = std::string((const char*)&fragmentIncludeBuffer[0], fragmentIncludeBufferByteCount);
        ma->Deallocate(fragmentIncludeBuffer);
    }*/

    const std::string finalVertexCppStr = CleanShaderSource(appendStr + vertexCppStrInclude + "\n" + vertexCppStr);
    const std::string finalFragmentCppStr = CleanShaderSource(appendStr + fragmentCppStrInclude + "\n" + fragmentCppStr);

    const char* vertexShaderStrPtr = finalVertexCppStr.c_str();
    const char* fragmentShaderStrPtr = finalFragmentCppStr.c_str();
    const GLint vertexShaderStrByteSize = strlen(vertexShaderStrPtr);
    const GLint fragmentShaderStrByteSize = strlen(fragmentShaderStrPtr);
    GLuint instance = glCreateProgram();
    const GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderStrPtr, &vertexShaderStrByteSize);
    glShaderSource(fragmentShader, 1, &fragmentShaderStrPtr, &fragmentShaderStrByteSize);
    glCompileShader(vertexShader);
    glCompileShader(fragmentShader);
    glAttachShader(instance, vertexShader);
    glAttachShader(instance, fragmentShader);
    glLinkProgram(instance);
    GLint success;
    glGetProgramiv(instance, GL_LINK_STATUS, &success);
    if (!success)
        Print("Error: can't link shader!");
    if (!glIsProgram(instance))
        Print("Error: invalid shader!");
    GLint logBufferByteSize = 0;
    GLchar logBuffer[1024] = {};
    glGetShaderInfoLog(vertexShader, 1024, &logBufferByteSize, &logBuffer[0]);
    if (logBufferByteSize > 0)
    {
        Print("Error: can't compile vertex shader!");
        Print(logBuffer);
    }
    logBufferByteSize = 0;
    glGetShaderInfoLog(fragmentShader, 1024, &logBufferByteSize, &logBuffer[0]);
    if (logBufferByteSize > 0)
    {
        Print("Error: can't compile fragment shader!");
        Print(logBuffer);
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return SGPUShaderResource::Invalid();
}

void triton::XGraphicsBackendOGL::BindShader(const SGPUShaderResource& shader)
{
    const GLuint shaderID = (GLuint)shader.instance;
    glUseProgram(shaderID);
}

void triton::XGraphicsBackendOGL::UnbindShader()
{
    glUseProgram(0);
}

void triton::XGraphicsBackendOGL::DestroyShader(const SGPUShaderResource& shader)
{
    glDeleteProgram(shader.instance);
}

void triton::XGraphicsBackendOGL::SetShaderUniform(
    const SGPUShaderResource& shader,
    const std::string& name,
    const glm::mat4& matrix
)
{
    glUniformMatrix4fv(glGetUniformLocation(shader.instance, name.c_str()), 1, GL_FALSE, &matrix[0][0]);
}

void triton::XGraphicsBackendOGL::SetShaderUniform(
    const SGPUShaderResource& shader,
    const std::string& name,
    types::usize count,
    const types::f32* values
)
{
    glUniform4fv(glGetUniformLocation(shader.instance, name.c_str()), count, &values[0]);
}

void triton::XGraphicsBackendOGL::SetShaderUniform(
    const SGPUShaderResource& shader,
    const std::string& name,
    types::u32 value
)
{
    glUniform1ui(glGetUniformLocation(shader.instance, name.c_str()), value);
}

void triton::XGraphicsBackendOGL::SetShaderUniform(
    const SGPUShaderResource& shader,
    const std::string& name,
    types::s32 value
)
{
    glUniform1i(glGetUniformLocation(shader.instance, name.c_str()), value);
}

void triton::XGraphicsBackendOGL::BindTextureNamed(
    const SGPUShaderResource& shader,
    const SGPUTextureResource& texture,
    const std::string& textureName,
    types::s32 slot
)
{
    if (slot == -1)
        slot = texture.slot;

    if (texture.dimension == ETextureDimension::Texture2D)
    {
        glUniform1i(glGetUniformLocation((GLuint)shader.instance, textureName.c_str()), slot);
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, (GLuint)texture.instance);
        glActiveTexture(GL_TEXTURE0);
    }
    else if (texture.dimension == ETextureDimension::Texture2DArray)
    {
        glUniform1i(glGetUniformLocation((GLuint)shader.instance, textureName.c_str()), slot);
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D_ARRAY, (GLuint)texture.instance);
        glActiveTexture(GL_TEXTURE0);
    }
}

triton::SGPUInputLayoutResource triton::XGraphicsBackendOGL::CreateInputLayout()
{
    GLuint instance = 0;
    glGenVertexArrays(1, (GLuint*)&instance);

    return SGPUInputLayoutResource(instance, 0);
}

void triton::XGraphicsBackendOGL::BindInputLayout(const SGPUInputLayoutResource& inputLayout)
{
    glBindVertexArray((GLuint)inputLayout.instance);
}

void triton::XGraphicsBackendOGL::BindRigidInputLayout()
{
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 48, (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 48, (void*)12);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 48, (void*)20);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 48, (void*)32);
}

void triton::XGraphicsBackendOGL::BindSkinnedInputLayout()
{
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 80, (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 80, (void*)12);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 80, (void*)20);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 80, (void*)32);
    glVertexAttribIPointer(4, 4, GL_UNSIGNED_INT, 80, (void*)48);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 80, (void*)64);
}

void triton::XGraphicsBackendOGL::UnbindInputLayout()
{
    glBindVertexArray(0);
}

void triton::XGraphicsBackendOGL::DestroyInputLayout(const SGPUInputLayoutResource& vertexArray)
{
    GLuint instance = vertexArray.instance;
    glDeleteVertexArrays(1, (GLuint*)&instance);
}

triton::SGPURenderPassResource triton::XGraphicsBackendOGL::CreateRenderPass()
{
    return SGPURenderPassResource(0, 0, 0, 0, cVector4(0.0f), 0.0f);
}

void triton::XGraphicsBackendOGL::BindRenderPass(const SGPURenderPassResource& renderPass)
{
}

void triton::XGraphicsBackendOGL::UnbindRenderPass(const SGPURenderPassResource& renderPass)
{
}

void triton::XGraphicsBackendOGL::BindDepthState(const SDepthState& blendMode)
{
    if (blendMode.useDepthTest == K_TRUE)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);

    if (blendMode.useDepthWrite == K_TRUE)
        glDepthMask(GL_TRUE);
    else
        glDepthMask(GL_FALSE);
}

void triton::XGraphicsBackendOGL::BindBlendState(const SBlendState& blendMode)
{
    for (usize i = 0; i < blendMode.factorCount; i++)
    {
        GLenum srcFactor = GL_ZERO;
        GLenum dstFactor = GL_ZERO;

        switch (blendMode.srcFactors[i])
        {
            case EBlendFactor::ONE: srcFactor = GL_ONE; break;
            case EBlendFactor::SRC_COLOR: srcFactor = GL_SRC_COLOR; break;
            case EBlendFactor::INV_SRC_COLOR: srcFactor = GL_ONE_MINUS_SRC_COLOR; break;
            case EBlendFactor::SRC_ALPHA: srcFactor = GL_SRC_ALPHA; break;
            case EBlendFactor::INV_SRC_ALPHA: srcFactor = GL_ONE_MINUS_SRC_ALPHA; break;
        }

        switch (blendMode.dstFactors[i])
        {
            case EBlendFactor::ONE: dstFactor = GL_ONE; break;
            case EBlendFactor::SRC_COLOR: dstFactor = GL_SRC_COLOR; break;
            case EBlendFactor::INV_SRC_COLOR: dstFactor = GL_ONE_MINUS_SRC_COLOR; break;
            case EBlendFactor::SRC_ALPHA: dstFactor = GL_SRC_ALPHA; break;
            case EBlendFactor::INV_SRC_ALPHA: dstFactor = GL_ONE_MINUS_SRC_ALPHA; break;
        }

        glBlendFunci(i, srcFactor, dstFactor);
    }
}

void triton::XGraphicsBackendOGL::SetViewport(const SViewport& viewport)
{
    glViewport(viewport.rect.GetX(), viewport.rect.GetY(), viewport.rect.GetZ(), viewport.rect.GetW());
}

triton::SGPURenderTargetResource triton::XGraphicsBackendOGL::CreateRenderTarget(
    const std::vector<SGPUTextureResource>& colorAttachments,
    const SGPUTextureResource& depthAttachment
)
{
    GLuint instance = 0;

    GLenum buffs[16] = {};
    glGenFramebuffers(1, (GLuint*)&instance);
    glBindFramebuffer(GL_FRAMEBUFFER, instance);
    for (usize i = 0; i < colorAttachments.size(); i++)
    {
        buffs[i] = GL_COLOR_ATTACHMENT0 + i;
        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0 + i,
            GL_TEXTURE_2D,
            colorAttachments[i].instance,
            0
        );
    }
    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_DEPTH_STENCIL_ATTACHMENT,
        GL_TEXTURE_2D,
        depthAttachment.instance,
        0
    );
    glDrawBuffers(colorAttachments.size(), &buffs[0]);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (status != GL_FRAMEBUFFER_COMPLETE)
        Print("Error: incomplete framebuffer!");

    return SGPURenderTargetResource(instance, 0, colorAttachments, depthAttachment);
}

void triton::XGraphicsBackendOGL::ResizeRenderTargetColors(
    SGPURenderTargetResource& renderTarget,
    const glm::vec2& size
)
{
    std::vector<SGPUTextureResource> newColorAttachments;
    for (usize i = 0; i < renderTarget.color_attachments.size(); i++)
    {
        auto& attachment = renderTarget.color_attachments[i];
        newColorAttachments.emplace_back(
            CreateTexture(
                cVector3(size.x, size.y, attachment.size.GetZ()),
                attachment.dimension,
                attachment.format,
                nullptr,
                0
            )
        );
        DestroyTexture(attachment);
    }
    //renderTarget.SetColorAttachments(newColorAttachments);

    GLenum buffs[16] = {};
    glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)renderTarget.instance);
    for (usize i = 0; i < renderTarget.color_attachments.size(); i++)
    {
        buffs[i] = GL_COLOR_ATTACHMENT0 + i;
        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0 + i,
            GL_TEXTURE_2D,
            (GLuint)renderTarget.color_attachments[i].instance,
            0
        );
    }
    glDrawBuffers(renderTarget.color_attachments.size(), &buffs[0]);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void triton::XGraphicsBackendOGL::ResizeRenderTargetDepth(
    SGPURenderTargetResource& renderTarget,
    const glm::vec2& size
)
{
    SGPUTextureResource newDepthAttachment = CreateTexture(
        cVector3(size.x, size.y, renderTarget.depth_attachment.size.GetZ()),
        renderTarget.depth_attachment.dimension,
        renderTarget.depth_attachment.format,
        nullptr,
        0
    );
    DestroyTexture(renderTarget.depth_attachment);
    renderTarget.depth_attachment = newDepthAttachment;

    GLenum buffs[16] = {};
    glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)renderTarget.instance);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_DEPTH_STENCIL_ATTACHMENT,
        GL_TEXTURE_2D,
        renderTarget.depth_attachment.instance,
        0
    );
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void triton::XGraphicsBackendOGL::UpdateRenderTargetBuffers(SGPURenderTargetResource& renderTarget)
{
    GLuint instance = 0;

    GLenum buffs[16] = {};
    glGenFramebuffers(1, &instance);
    glBindFramebuffer(GL_FRAMEBUFFER, instance);
    for (usize i = 0; i < renderTarget.color_attachments.size(); i++)
    {
        buffs[i] = GL_COLOR_ATTACHMENT0 + i;
        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0 + i,
            GL_TEXTURE_2D,
            (GLuint)renderTarget.color_attachments[i].instance,
            0
        );
    }
    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_DEPTH_STENCIL_ATTACHMENT,
        GL_TEXTURE_2D,
        (GLuint)renderTarget.depth_attachment.instance,
        0
    );
    glDrawBuffers(renderTarget.color_attachments.size(), &buffs[0]);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void triton::XGraphicsBackendOGL::BindRenderTarget(const SGPURenderTargetResource& renderTarget)
{
    glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)renderTarget.instance);
}

void triton::XGraphicsBackendOGL::UnbindRenderTarget()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void triton::XGraphicsBackendOGL::DestroyRenderTarget(const SGPURenderTargetResource& renderTarget)
{
    UnbindRenderTarget();

    GLuint instance = renderTarget.instance;
    glDeleteFramebuffers(1, &instance);
}

triton::SGPUBufferResource triton::XGraphicsBackendOGL::CreateBuffer(
    EGPUBufferType type,
    const types::u8* data,
    types::usize byteSize,
    types::s32 slot
)
{
    GLuint instance = 0;

    glGenBuffers(1, (GLuint*)&instance);

    if (type == EGPUBufferType::Vertex)
    {
        glBindBuffer(GL_ARRAY_BUFFER, instance);
        glBufferData(GL_ARRAY_BUFFER, byteSize, data, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    else if (type == EGPUBufferType::Index)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, instance);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, byteSize, data, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    else if (type == EGPUBufferType::Uniform)
    {
        glBindBuffer(GL_UNIFORM_BUFFER, instance);
        glBufferData(GL_UNIFORM_BUFFER, byteSize, data, GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
    else if (type == EGPUBufferType::Storage)
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, instance);
        glBufferData(GL_SHADER_STORAGE_BUFFER, byteSize, data, GL_STATIC_DRAW);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    return SGPUBufferResource(instance, 0, 0, type, byteSize, slot);
}

void triton::XGraphicsBackendOGL::BindBuffer(const SGPUBufferResource& buffer)
{
    if (buffer.type == EGPUBufferType::Vertex)
        glBindBuffer(GL_ARRAY_BUFFER, (GLuint)buffer.instance);
    else if (buffer.type == EGPUBufferType::Index)
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (GLuint)buffer.instance);
    else if (buffer.type == EGPUBufferType::Uniform)
        glBindBufferBase(GL_UNIFORM_BUFFER, buffer.slot, (GLuint)buffer.instance);
    else if (buffer.type == EGPUBufferType::Storage)
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, buffer.slot, buffer.instance);
}

void triton::XGraphicsBackendOGL::BindBufferNotVAO(const SGPUBufferResource& buffer)
{
    if (buffer.type == EGPUBufferType::Uniform)
        glBindBufferBase(GL_UNIFORM_BUFFER, buffer.slot, (GLuint)buffer.instance);
    else if (buffer.type == EGPUBufferType::Storage)
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, buffer.slot, buffer.instance);
}

void triton::XGraphicsBackendOGL::UnbindBuffer(const SGPUBufferResource& buffer)
{
    if (buffer.type == EGPUBufferType::Vertex)
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    else if (buffer.type == EGPUBufferType::Index)
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    else if (buffer.type == EGPUBufferType::Uniform)
        glBindBufferBase(GL_UNIFORM_BUFFER, buffer.slot, 0);
    else if (buffer.type == EGPUBufferType::Storage)
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, buffer.slot, 0);
}

void triton::XGraphicsBackendOGL::WriteBuffer(
    const SGPUBufferResource& buffer,
    types::usize offset,
    types::usize byteSize,
    const types::u8* data
)
{
    if (buffer.type == EGPUBufferType::Vertex)
    {
        glBindBuffer(GL_ARRAY_BUFFER, buffer.instance);
        glBufferSubData(GL_ARRAY_BUFFER, offset, byteSize, data);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    else if (buffer.type == EGPUBufferType::Index)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.instance);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, byteSize, data);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    else if (buffer.type == EGPUBufferType::Uniform)
    {
        glBindBuffer(GL_UNIFORM_BUFFER, buffer.instance);
        glBufferSubData(GL_UNIFORM_BUFFER, offset, byteSize, data);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
    else if (buffer.type == EGPUBufferType::Storage)
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer.instance);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, byteSize, data);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }
}

void triton::XGraphicsBackendOGL::DestroyBuffer(const SGPUBufferResource& buffer)
{
    if (buffer.type == EGPUBufferType::Vertex)
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    else if (buffer.type == EGPUBufferType::Index)
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    else if (buffer.type == EGPUBufferType::Uniform)
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    else if (buffer.type == EGPUBufferType::Storage)
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    GLuint instance = buffer.instance;
    glDeleteBuffers(1, &instance);
}

triton::SGPUTextureResource triton::XGraphicsBackendOGL::CreateTexture(
    const cVector3& size,
    ETextureDimension dimension,
    ETextureFormat format,
    const types::u8* data,
    types::s32 slot
)
{
    GLuint instance = 0;

    glGenTextures(1, (GLuint*)&instance);

    GLenum formatGL = GL_RGBA8;
    GLenum channelsGL = GL_RGBA;
    GLenum formatComponentGL = GL_UNSIGNED_BYTE;
    if (format == ETextureFormat::R8)
    {
        formatGL = GL_R8;
        channelsGL = GL_RED;
        formatComponentGL = GL_UNSIGNED_BYTE;
    }
    else if (format == ETextureFormat::R8F)
    {
        formatGL = GL_R8;
        channelsGL = GL_RED;
        formatComponentGL = GL_FLOAT;
    }
    else if (format == ETextureFormat::RGBA8)
    {
        formatGL = GL_RGBA8;
        channelsGL = GL_RGBA;
        formatComponentGL = GL_UNSIGNED_BYTE;
    }
    else if (format == ETextureFormat::RGBA8_SRGB_Mips)
    {
        formatGL = GL_SRGB8_ALPHA8;
        channelsGL = GL_RGBA;
        formatComponentGL = GL_UNSIGNED_BYTE;
    }
    else if (format == ETextureFormat::RGB16F)
    {
        formatGL = GL_RGB16F;
        channelsGL = GL_RGB;
        formatComponentGL = GL_HALF_FLOAT;
    }
    else if (format == ETextureFormat::RGBA16F)
    {
        formatGL = GL_RGBA16F;
        channelsGL = GL_RGBA;
        formatComponentGL = GL_HALF_FLOAT;
    }
    else if (format == ETextureFormat::DepthStencil)
    {
        formatGL = GL_DEPTH24_STENCIL8;
        channelsGL = GL_DEPTH_STENCIL;
        formatComponentGL = GL_UNSIGNED_INT_24_8;
    }

    if (dimension == ETextureDimension::Texture2D)
    {
        glBindTexture(GL_TEXTURE_2D, instance);

        glTexImage2D(GL_TEXTURE_2D, 0, formatGL, size.GetX(), size.GetY(), 0, channelsGL, formatComponentGL, data);
        if (format != ETextureFormat::DepthStencil)
        {
            if (format == ETextureFormat::RGBA8_SRGB_Mips)
            {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            }
            else
            {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            }
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }

        glBindTexture(GL_TEXTURE_2D, 0);
    }
    else if (dimension == ETextureDimension::Texture2DArray)
    {
        glBindTexture(GL_TEXTURE_2D_ARRAY, instance);

        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, formatGL, size.GetX(), size.GetY(), size.GetZ(), 0, channelsGL, formatComponentGL, data);

        if (format == ETextureFormat::RGBA8_SRGB_Mips)
        {
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
        else
        {
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    }

    return SGPUTextureResource::Invalid();
}

triton::SGPUTextureResource triton::XGraphicsBackendOGL::ResizeTexture(const SGPUTextureResource& texture, const cVector2& size)
{
    SGPUTextureResource newTexture = CreateTexture(
        cVector3(size.GetX(), size.GetY(), texture.size.GetZ()),
        texture.dimension,
        texture.format,
        nullptr,
        texture.slot
    );
    DestroyTexture(texture);

    return newTexture;
}

void triton::XGraphicsBackendOGL::BindTexture(const SGPUTextureResource& texture)
{
    if (texture.dimension == ETextureDimension::Texture2D)
    {
        glActiveTexture(GL_TEXTURE0 + texture.slot);
        glBindTexture(GL_TEXTURE_2D, texture.instance);
        glActiveTexture(GL_TEXTURE0);
    }
    else if (texture.dimension == ETextureDimension::Texture2DArray)
    {
        glActiveTexture(GL_TEXTURE0 + texture.slot);
        glBindTexture(GL_TEXTURE_2D_ARRAY, texture.instance);
        glActiveTexture(GL_TEXTURE0);
    }
}

void triton::XGraphicsBackendOGL::UnbindTexture(const SGPUTextureResource& texture)
{
    if (texture.dimension == ETextureDimension::Texture2D)
        glBindTexture(GL_TEXTURE_2D, 0);
    else if (texture.dimension == ETextureDimension::Texture2DArray)
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void triton::XGraphicsBackendOGL::WriteTexture(
    const SGPUTextureResource& texture,
    const cVector3& offset,
    const cVector2& size,
    const types::u8* data
)
{
    GLenum formatGL = GL_RGBA8;
    GLenum channelsGL = GL_RGBA;
    GLenum formatComponentGL = GL_UNSIGNED_BYTE;

    if (texture.format == ETextureFormat::R8)
    {
        formatGL = GL_R8;
        channelsGL = GL_RED;
        formatComponentGL = GL_UNSIGNED_BYTE;
    }
    else if (texture.format == ETextureFormat::R8F)
    {
        formatGL = GL_R8;
        channelsGL = GL_RED;
        formatComponentGL = GL_FLOAT;
    }
    else if (texture.format == ETextureFormat::RGBA8_SRGB_Mips)
    {
        formatGL = GL_SRGB8_ALPHA8;
        channelsGL = GL_RGBA;
        formatComponentGL = GL_UNSIGNED_BYTE;
    }
    else if (texture.format == ETextureFormat::RGBA8)
    {
        formatGL = GL_RGBA8;
        channelsGL = GL_RGBA;
        formatComponentGL = GL_UNSIGNED_BYTE;
    }
    else if (texture.format == ETextureFormat::RGB16F)
    {
        formatGL = GL_RGB16F;
        channelsGL = GL_RGB;
        formatComponentGL = GL_HALF_FLOAT;
    }
    else if (texture.format == ETextureFormat::RGBA16F)
    {
        formatGL = GL_RGBA16F;
        channelsGL = GL_RGBA;
        formatComponentGL = GL_HALF_FLOAT;
    }
    else if (texture.format == ETextureFormat::DepthStencil)
    {
        formatGL = GL_DEPTH24_STENCIL8;
        channelsGL = GL_DEPTH_STENCIL;
        formatComponentGL = GL_UNSIGNED_INT_24_8;
    }

    if (texture.dimension == ETextureDimension::Texture2D)
    {
        glBindTexture(GL_TEXTURE_2D, texture.instance);
        glTexSubImage2D(
            GL_TEXTURE_2D,
            0,
            offset.GetX(),
            offset.GetY(),
            size.GetX(),
            size.GetY(),
            channelsGL,
            formatComponentGL,
            data
        );
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    else if (texture.dimension == ETextureDimension::Texture2DArray)
    {
        if (offset.GetX() + size.GetX() <= texture.size.GetX() &&
            offset.GetY() + size.GetY() <= texture.size.GetY() &&
            offset.GetZ() < texture.size.GetZ())
        {
            glBindTexture(GL_TEXTURE_2D_ARRAY, texture.instance);
            glTexSubImage3D(
                GL_TEXTURE_2D_ARRAY,
                0,
                offset.GetX(),
                offset.GetY(),
                offset.GetZ(),
                size.GetX(),
                size.GetY(),
                1,
                channelsGL,
                formatComponentGL,
                data
            );
            glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
        }
    }
}

void triton::XGraphicsBackendOGL::WriteTextureToFile(const SGPUTextureResource& texture, const std::string& filename)
{
    if (texture.format != ETextureFormat::RGBA8)
        return;

    GLenum channelsGL = GL_RGBA;
    GLenum formatComponentGL = GL_UNSIGNED_BYTE;
    usize formatByteCount = 4;

    if (texture.format == ETextureFormat::RGBA8)
    {
        channelsGL = GL_RGBA;
        formatComponentGL = GL_UNSIGNED_BYTE;
        formatByteCount = 4;
    }

    if (texture.dimension == ETextureDimension::Texture2D)
    {
        const SCapabilities& caps = _context->GetSubsystem<XEngine>()->GetApplication()->GetCapabilities();

        SMemAlloc pixels = CMemoryAllocator::Allocate(
            texture.size.GetX() * texture.size.GetY() * formatByteCount
        );

        glBindTexture(GL_TEXTURE_2D, texture.instance);
        glGetTexImage(GL_TEXTURE_2D, 0, channelsGL, formatComponentGL, pixels.mem);
        glBindTexture(GL_TEXTURE_2D, 0);

        // TODO: Implement texture to file write

        CMemoryAllocator::Deallocate(pixels);
    }
}

void triton::XGraphicsBackendOGL::GenerateTextureMips(const SGPUTextureResource& texture)
{
    if (texture.dimension == ETextureDimension::Texture2D)
    {
        glBindTexture(GL_TEXTURE_2D, texture.instance);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    else if (texture.dimension == ETextureDimension::Texture2DArray)
    {
        glBindTexture(GL_TEXTURE_2D_ARRAY, texture.instance);
        glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    }
}

void triton::XGraphicsBackendOGL::DestroyTexture(const SGPUTextureResource& texture)
{
    if (texture.dimension == ETextureDimension::Texture2D)
        glBindTexture(GL_TEXTURE_2D, 0);
    else if (texture.dimension == ETextureDimension::Texture2DArray)
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    GLuint instance = texture.instance;
    glDeleteTextures(1, (GLuint*)&instance);
}