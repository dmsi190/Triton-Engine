// render_thread.cpp

#include <cstring>
#include <tracy/Tracy.hpp>
#include "render_thread.hpp"
#include "context.hpp"
#include "engine.hpp"
#include "window.hpp"
#include "graphics.hpp"
#include "graphics_backend2.hpp"
#include "thread_guard.hpp"
#include "input_backend.hpp"
#include "render_native_command_struct.hpp"
#include "render_native_draw_command_info_struct.hpp"
#include "render_instance_motion_type.hpp"
#include "texture_format.hpp"

using namespace types;

void triton::XRenderThread::ThreadFunction()
{
	std::mutex mtx;
	CThreadGuard::CaptureRenderThreadId();

	tracy::SetThreadName("Render Thread");

	IGraphicsBackend2* gfxBackend = _context->GetBackend<IGraphicsBackend2>();

	// Create graphics contexts for windows
	IApplication* app = _context->GetSubsystem<CEngine>()->GetApplication();
	CWindow* window = app->GetWindow();
	
	const SCapabilities& caps = _context->GetSubsystem<CEngine>()->GetApplication()->GetCapabilities();

	gfxBackend->Initialize(
		window->GetBackendWindow(),
		True,
		_context->GetBackend<IInputBackend>()->GetBackendWindowVulkanExtensions(),
		caps.preferredGraphicsDevice,
		caps.staging_buffer_byte_size,
		window->GetSize(),
		app->GetCapabilities().framesInFlight
	);

	_sync->InitRenderThread();
	
	{
		std::lock_guard<std::mutex> lg(mtx);
		std::cout << "Render thread ready and starts\n\n";
	}

	while (K_TRUE)
	{
		//_synchronization->LoopStart();
		//_renderSubsystem->NotifyMainThread();

		{
			ZoneScopedN("Wait for Produced Frame");

			_sync->WaitForProducedFrame();
		}

		boolean bDoExit = K_FALSE;
		{
			ZoneScopedN("Render Job");

			const SProducedFrameData& frame = _sync->GetProducedFrameData();

			if (frame.operation == EProducedFrameOp::Kill)
			{
				bDoExit = K_TRUE;
			}
			else if (frame.operation == EProducedFrameOp::ExecuteFull)
			{
				// Full job
				gfxBackend->BeginFrame();
				ExecuteCommands(
					frame.renderCommandPack,
					gfxBackend
				);
				gfxBackend->EndFrame();
			}
			else if (frame.operation == EProducedFrameOp::ExecuteCommandsOnly)
			{
				// Execute render commands only
				ExecuteCommands(
					frame.renderCommandPack,
					gfxBackend
				);
			}
		}
		
		_sync->FinishRenderJob();

		{
			ZoneScopedN("Release Frame");

			_sync->ReleaseFrame();
		}

		if (bDoExit == K_TRUE)
			break;
	}
}

void triton::XRenderThread::ExecuteCommands(
	const SRenderCommandPack& renderCommandPack,
	IGraphicsBackend2* gfxBackend
)
{
	for (usize i = 0; i < renderCommandPack.count; i++)
	{
		const SRenderCommand& cmd = renderCommandPack.cmds[i];

		switch (cmd._command)
		{
			case ERenderCommand::RESIZE_RENDER_TARGETS:
			{
				/*gfx->ResizeRenderTargets(cVector2(
					cmd._args._argA,
					cmd._args._argB
				));*/
				break;
			}
			case ERenderCommand::CLEAR:
			{
				/*gfxBackend->ClearColor(cVector4(
					cmd._args._argA,
					cmd._args._argB,
					cmd._args._argC,
					cmd._args._argD
				));
				gfxBackend->ClearDepth(
					cmd._args._argE
				);*/
				break;
			}
			case ERenderCommand::DRAW:
			{
				/*gfxBackend->Draw(
					cmd._args._argA,
					cmd._args._argB,
					cmd._args._argC,
					cmd._args._argD
				);*/
				break;
			}
			case ERenderCommand::CREATE_BUFFER:
			{
				SGPUBufferResource resultBuffer = gfxBackend->CreateBuffer(
					(EGPUBufferType)cmd._args._argA,
					cmd._args._argB
				);
				new (_sync->GetResultBuffer().data.mem) SGPUBufferResource(resultBuffer);
				break;
			}
			case ERenderCommand::BIND_BUFFER:
			{
				//CGPUBufferResource* buffer = (CGPUBufferResource*)cmd._args._argA;
				//gfxBackend->BindBuffer(*buffer);
				break;
			}
			case ERenderCommand::WRITE_BUFFER:
			{
				gfxBackend->WriteBuffer(
					*((SGPUBufferResource*)cmd._args._argA),
					(usize)cmd._args._argB,
					(const u8*)cmd._args._argC,
					(usize)cmd._args._argD
				);
				break;
			}
			case ERenderCommand::DESTROY_BUFFER:
			{
				gfxBackend->DestroyBuffer(
					*((SGPUBufferResource*)cmd._args._argA)
				);
				break;
			}
			case ERenderCommand::CREATE_INPUT_LAYOUT:
			{
				//CGPUInputLayoutResource resultIA = gfxBackend->CreateInputLayout();
				//memcpy(&_sync->GetResultBuffer().data[0], &resultIA, sizeof(CGPUInputLayoutResource));
				break;
			}
			case ERenderCommand::BIND_INPUT_LAYOUT:
			{
				//CGPUInputLayoutResource* ia = (CGPUInputLayoutResource*)cmd._args._argA;
				//gfxBackend->BindInputLayout(*ia);
				break;
			}
			case ERenderCommand::UNBIND_INPUT_LAYOUT:
			{
				//gfxBackend->UnbindInputLayout();
				break;
			}
			case ERenderCommand::DESTROY_INPUT_LAYOUT:
			{
				//CGPUInputLayoutResource* ia = (CGPUInputLayoutResource*)cmd._args._argA;
				//gfxBackend->DestroyInputLayout(*ia);
				break;
			}
			case ERenderCommand::CREATE_TEXTURE:
			{
				SGPUTextureResource resultTexture = gfxBackend->CreateTexture(
					cmd._args._argA,
					(ETextureFormat)cmd._args._argB,
					(dword)cmd._args._argC,
					(ETextureDimension)cmd._args._argD,
					*((const cVector3*)cmd._args._argE),
					(SGPUTextureResource*)cmd._args._argF
				);
				new (_sync->GetResultBuffer().data.mem) SGPUTextureResource(resultTexture);
				break;
			}
			case ERenderCommand::WRITE_TEXTURE:
			{
				gfxBackend->WriteTexture(
					*((SGPUTextureResource*)cmd._args._argA),
					(u8*)cmd._args._argB
				);
				break;
			}
			case ERenderCommand::GENERATE_TEXTURE_MIPS:
			{
				/*gfxBackend->GenerateTextureMips(
					*((CGPUTextureResource*)cmd._args._argA)
				);*/
				break;
			}
			case ERenderCommand::DESTROY_TEXTURE:
			{
				gfxBackend->DestroyTexture(
					*((SGPUTextureResource*)cmd._args._argA)
				);
				break;
			}
			case ERenderCommand::CREATE_RENDER_TARGET:
			{
				usize attachmentCount = cmd._args._argA;
				SGPUTextureResource* attachments = (SGPUTextureResource*)cmd._args._argB;
				std::vector<SGPUTextureResource> attachmentsVec;
				for (usize i = 0; i < attachmentCount; i++)
					attachmentsVec.push_back(attachments[i]);
				SGPURenderTargetResource resultRenderTarget = gfxBackend->CreateRenderTarget(
					attachmentsVec,
					*((SGPUTextureResource*)cmd._args._argC)
				);
				new (_sync->GetResultBuffer().data.mem) SGPURenderTargetResource(resultRenderTarget);
				break;
			}
			case ERenderCommand::CREATE_SHADER:
			{
				SGPUShaderResource resultGPUShader = gfxBackend->CreateShader(
					(dword)cmd._args._argA,
					*((SShaderBytecodeFiles*)cmd._args._argB)
				);
				new (_sync->GetResultBuffer().data.mem) SGPUShaderResource(resultGPUShader);
				break;
			}
			case ERenderCommand::DESTROY_SHADER:
			{
				gfxBackend->DestroyShader(
					*((SGPUShaderResource*)cmd._args._argA)
				);
				break;
			}
			case ERenderCommand::BIND_RIGID_INPUT_LAYOUT:
			{
				//gfxBackend->BindRigidInputLayout();
				break;
			}
			case ERenderCommand::BIND_SKINNED_INPUT_LAYOUT:
			{
				//gfxBackend->BindSkinnedInputLayout();
				break;
			}
			case ERenderCommand::CREATE_RENDER_PASS:
			{
				SGPURenderPassResource resultRenderPass = gfxBackend->CreateRenderPass(
					*((SGPURenderTargetResource*)cmd._args._argA),
					cmd._args._argB,
					*(cVector4*)cmd._args._argC,
					*(f32*)cmd._args._argD,
					*(std::vector<ERenderResourceUsage>*)cmd._args._argE,
					*(std::vector<ERenderResourceUsage>*)cmd._args._argF
				);
				memcpy(&_sync->GetResultBuffer().data.mem[0], &resultRenderPass, sizeof(SGPURenderPassResource));
				break;
			}
			case ERenderCommand::BeginRenderPass:
			{
				gfxBackend->AddCommandToBuffer(
					ENativeRenderCommand::BeginRenderPass,
					(const void*)cmd._args._argA,
					(const void*)cmd._args._argB
				);
				gfxBackend->AddCommandToBuffer(
					ENativeRenderCommand::BindPipeline,
					(const void*)cmd._args._argC,
					nullptr
				);
				gfxBackend->AddCommandToBuffer(
					ENativeRenderCommand::BindVertexBuffer,
					(const void*)cmd._args._argD,
					nullptr
				);
				gfxBackend->AddCommandToBuffer(
					ENativeRenderCommand::BindIndexBuffer,
					(const void*)cmd._args._argE,
					nullptr
				);
				gfxBackend->AddCommandToBuffer(
					ENativeRenderCommand::PushConstants,
					(const void*)cmd._args._argF,
					(const void*)cmd._args._argC
				);
				gfxBackend->AddCommandToBuffer(
					ENativeRenderCommand::BindDescriptorSets,
					(const void*)cmd._args._argG,
					(const void*)cmd._args._argC
				);
				break;
			}
			case ERenderCommand::ExecuteRenderPass:
			{
				const usize bindingGroupCount = cmd._args._argA;
				SGPUBindingGroupResource* bindingGroups = (SGPUBindingGroupResource*)cmd._args._argB;
				std::vector<SGPUBindingGroupResource> bindingGroupsVector;
				for (usize i = 0; i < bindingGroupCount; ++i)
					bindingGroupsVector.push_back(bindingGroups[i]);

				SNativeCommandDrawInfo drawInfo = {};
				drawInfo.instanceMotionType = (ERenderInstanceMotionType)cmd._args._argD;
				drawInfo.vertexCount = cmd._args._argE;
				drawInfo.indexCount = cmd._args._argF;
				drawInfo.instanceCount = cmd._args._argG;
				drawInfo.firstIndex = cmd._args._argH;
				drawInfo.baseVertex = cmd._args._argI;
				drawInfo.firstInstance = cmd._args._argJ;

				gfxBackend->AddCommandToBuffer(
					ENativeRenderCommand::BindDescriptorSets,
					(const void*)&bindingGroupsVector,
					(const void*)cmd._args._argC
				);
				gfxBackend->AddCommandToBuffer(
					ENativeRenderCommand::Draw,
					(const void*)&drawInfo,
					(const void*)cmd._args._argC
				);

				break;
			}
			case ERenderCommand::EndRenderPass:
			{
				gfxBackend->AddCommandToBuffer(
					ENativeRenderCommand::EndRenderPass,
					nullptr,
					nullptr
				);
				break;
			}
			case ERenderCommand::DESTROY_RENDER_PASS:
			{
				gfxBackend->DestroyRenderPass(
					*((SGPURenderPassResource*)cmd._args._argA)
				);
				break;
			}
			case ERenderCommand::CREATE_PIPELINE:
			{
				SGPUPipelineResource resultPipeline = gfxBackend->CreatePipeline(
					*(SGPUShaderResource*)cmd._args._argA,
					*(std::vector<SGPUBindingGroupLayoutResource>*)cmd._args._argB,
					(boolean)cmd._args._argC,
					cmd._args._argD ? std::optional(std::reference_wrapper(*(SViewport*)cmd._args._argD)) : std::nullopt,
					cmd._args._argE ? std::optional(std::reference_wrapper(*(SGPURenderTargetResource*)cmd._args._argE)) : std::nullopt,
					cmd._args._argF ? std::optional(std::reference_wrapper(*(SGPURenderPassResource*)cmd._args._argF)) : std::nullopt,
					(ERenderPrimitiveTopology)cmd._args._argG,
					(EVertexBufferFormat)cmd._args._argH
				);
				
				new (_sync->GetResultBuffer().data.mem) SGPUPipelineResource(resultPipeline);

				break;
			}
			case ERenderCommand::DESTROY_PIPELINE:
			{
				gfxBackend->DestroyPipeline(
					*(SGPUPipelineResource*)cmd._args._argA
				);
				break;
			}
			case ERenderCommand::CreateBindingGroupLayout:
			{
				SGPUBindingGroupLayoutResource resultBindingGroupLayout = gfxBackend->CreateBindingGroupLayout(
					*(const std::vector<SRenderBindingGroupBinding>*)cmd._args._argA
				);

				new (_sync->GetResultBuffer().data.mem) SGPUBindingGroupLayoutResource(resultBindingGroupLayout);

				break;
			}
			case ERenderCommand::DestroyBindingGroupLayout:
			{
				gfxBackend->DestroyBindingGroupLayout(
					*(SGPUBindingGroupLayoutResource*)cmd._args._argA
				);
				break;
			}
			case ERenderCommand::CreateBindingGroup:
			{
				SGPUBindingGroupResource resultBindingGroup = gfxBackend->CreateBindingGroup(
					cmd._args._argA,
					*(const SGPUBindingGroupLayoutResource*)cmd._args._argB,
					*(const std::vector<SRenderBindingGroupBinding>*)cmd._args._argC,
					*(const std::vector<SRenderBindingGroupBinding>*)cmd._args._argD
				);

				new (_sync->GetResultBuffer().data.mem) SGPUBindingGroupResource(resultBindingGroup);

				break;
			}
			case ERenderCommand::DestroyBindingGroup:
			{
				gfxBackend->DestroyBindingGroup(
					*(SGPUBindingGroupResource*)cmd._args._argA
				);
				break;
			}
			case ERenderCommand::FinalizeSwapchain:
			{
				gfxBackend->FinalizeSwapchain(
					*(SGPUTextureResource*)cmd._args._argA
				);
				break;
			}
			case ERenderCommand::ReleaseSwapchainResources:
			{
				gfxBackend->ReleaseSwapchainResources();
				break;
			}
			case ERenderCommand::BindVertexBuffer:
			{
				gfxBackend->AddCommandToBuffer(
					ENativeRenderCommand::BindVertexBuffer,
					(const void*)cmd._args._argA,
					nullptr
				);
				break;
			}
			case ERenderCommand::BindIndexBuffer:
			{
				gfxBackend->AddCommandToBuffer(
					ENativeRenderCommand::BindIndexBuffer,
					(const void*)cmd._args._argA,
					nullptr
				);
				break;
			}
			case ERenderCommand::Dispatch:
			{
				gfxBackend->Dispatch(
					*(SGPUPipelineResource*)cmd._args._argA,
					*(const std::vector<SGPUBindingGroupResource>*)cmd._args._argB,
					cVector3(cmd._args._argC, cmd._args._argD, cmd._args._argE)
				);
				break;
			}
		}
	}
}

void triton::XRenderThread::Present(IGraphicsBackend2* gfxBackend)
{
	//gfxBackend->SwapWindowBuffers(_window->GetBackendWindow());
	gfxBackend->BeginFrame();
	gfxBackend->EndFrame();
}