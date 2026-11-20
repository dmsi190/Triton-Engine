// shader.hpp

#pragma once

#include <string>
#include <vector>
#include "object.hpp"
#include "gpu_shader_resource.hpp"
#include "shader_define.hpp"
#include "shader_bytecode_files_struct.hpp"
#include "shader_stage_bit_enum.hpp"
#include "types.hpp"

namespace triton
{
	class XShader : public iObject
	{
		TRITON_OBJECT(XShader)

		SGPUShaderResource _gpu_shader = SGPUShaderResource::Invalid();

	public:
		explicit XShader(
			const SMemAlloc& mem_alloc,
			cContext* context,
			EShaderStageBit stageMask,
			const SShaderBytecodeFiles& bytecodeFiles
		);

		~XShader() override;

		inline const SGPUShaderResource& GetGPUResource() const { return _gpu_shader; }
	};
}