// shader.cpp

#include "shader.hpp"
#include "context.hpp"
#include "engine.hpp"
#include "synchronization.hpp"
#include "graphics_backend.hpp"
#include "thread_guard.hpp"
#include "render_command_proxy.hpp"

using namespace types;

triton::XShader::XShader(
	const SMemAlloc& mem_alloc,
	cContext* context,
	EShaderStageBit stageMask,
	const SShaderBytecodeFiles& bytecodeFiles
) : iObject(mem_alloc, context)
{
	CThreadGuard::AssertMain();

	_gpu_shader = CRenderCommandProxy::CreateShader(
		stageMask,
		bytecodeFiles
	);
}

triton::XShader::~XShader()
{
	CThreadGuard::AssertMain();

	CRenderCommandProxy::DestroyShader(_gpu_shader);
}