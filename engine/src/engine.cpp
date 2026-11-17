// engine.cpp

#include <tracy/Tracy.hpp>
#include "engine.hpp"
#include "creator.hpp"
#include "application.hpp"
#include "context.hpp"
#include "graphics.hpp"
#include "filesystem_manager.hpp"
#include "font_manager.hpp"
#include "physics_manager.hpp"
#include "time.hpp"
#include "event_manager.hpp"
#include "thread_subsystem.hpp"
#include "audio.hpp"
#include "math.hpp"
#include "input_backend_sdl.hpp"
#include "graphics_backend_ogl.hpp"
#include "audio_backend_oal.hpp"
#include "render_thread.hpp"
#include "thread_guard.hpp"
#include "geometry_storage.hpp"
#include "model3d_backend.hpp"
#include "model3d_backend_assimp.hpp"
#include "synchronization.hpp"
#include "render_thread.hpp"
#include "camera_pool.hpp"
#include "skinned_bones_pool.hpp"
#include "model3d_pool.hpp"
#include "skin_pool.hpp"
#include "input_layout_pool.hpp"
#include "render_target_pool.hpp"
#include "render_pass_pools.hpp"
#include "shader_pool.hpp"
#include "gpu_buffer_pool.hpp"
#include "render_instance_static_pool.hpp"
#include "render_instance_dynamic_pool.hpp"
#include "texture_pool.hpp"
#include "world_render_domain_pool.hpp"
#include "world_render_group_pool.hpp"
#include "world_object_pool.hpp"
#include "window.hpp"
#include "image_decoder_multi.hpp"
#include "graphics_backend2_vulkan.hpp"
#include "world.hpp"
#include "render_command_proxy.hpp"
#include "graphics_buffers.hpp"

using namespace types;

triton::CEngine::CEngine(cContext* context, IApplication* app) : CSubsystem(context), _app(app)
{
	Initialize();
}

triton::CEngine::~CEngine()
{
	Shutdown();
}

void triton::CEngine::Initialize()
{
	CThreadGuard::CaptureMainThreadId();

	const SCapabilities& caps = _app->GetCapabilities();

	// NOTE: The order of the objects registered below matters

	// Register Engine as subsystem
	_context->RegisterSubsystem(this);

	// Set context for render command proxy
	CRenderCommandProxy::SetContext(_context);

	// Create memory allocator
	CMemoryAllocator::Initialize(
		caps.memory_allocator_bin_count,
		caps.memory_allocator_bin_capacity,
		caps.memory_allocator_bin_mem_step
	);

	// Register backends
	_context->RegisterBackend<IInputBackend>(new CInputBackendSDL(_context));
	_context->RegisterBackend<iAudioBackend>(new cAudioBackendOAL(_context));
	_context->RegisterBackend<IModel3DBackend>(new XModel3DBackendAssimp(_context));
	_context->RegisterBackend<IImageDecoder>(new BImageDecoderMulti(_context));

	// Create application window
	_app->CreateWindow();

	// Register file system subsystem before graphics backend
	_context->RegisterSubsystem(new CFileSystem(_context));

	// Register graphics backend
	_context->RegisterBackend<IGraphicsBackend2>(new BGraphicsBackend2Vulkan(_context));

	// Initialize render thread
	InitializeRenderCommandRecorder();
	InitializeSynchronization();
	InitializeRenderThread();

	// Initialize graphics buffers
	CGraphicsBuffers::Initialize(_context);

	// Register subsystems
	_context->RegisterSubsystem(new CGeometryStorage(_context));

	_context->RegisterSubsystem(new CGraphics(_context));

	_context->RegisterSubsystem(new UWorld(_context));
}

void triton::CEngine::Shutdown()
{
	CGraphicsBuffers::Release();
	ReleaseRenderThread();
	ReleaseSynchronization();
	ReleaseRenderCommandRecorder();
	CMemoryAllocator::Release();
}

void triton::CEngine::Run()
{
	MainThreadFunction();
}

void triton::CEngine::InitializeSynchronization()
{
	_sync = CCreator::Create<XSynchronization>(_context, _cmd_recorder, 1024);
}

void triton::CEngine::InitializeRenderCommandRecorder()
{
	CThreadGuard::AssertMain();

	_cmd_recorder = CCreator::Create<XRenderCommandRecorder>();
}

void triton::CEngine::InitializeRenderThread()
{
	CThreadGuard::AssertMain();

	// Create render thread
	_render_thread = CCreator::Create<cRenderThread>(_context, _sync, _app->GetWindow());
	_render_thread->Run();

	// Wait until render thread gets initialized
	_sync->WaitForRenderThreadToInit();
}

void triton::CEngine::ReleaseSynchronization()
{
	CCreator::Destroy<XSynchronization>(_sync);
}

void triton::CEngine::ReleaseRenderCommandRecorder()
{
	CCreator::Destroy<XRenderCommandRecorder>(_cmd_recorder);
}

void triton::CEngine::ReleaseRenderThread()
{
	_render_thread->Stop();

	CCreator::Destroy<cRenderThread>(_render_thread);
}

void triton::CEngine::MainThreadFunction()
{
	CThreadGuard::AssertMain();

	std::cout << "Main thread started\n";

	tracy::SetThreadName("Main Thread");

	if (!_app)
		return;

	_app->Setup();

	boolean bIsRunning = True;

	IInputBackend* inputBackend = _context->GetBackend<IInputBackend>();
	CWindow* window = _app->GetWindow();

	while (bIsRunning)
	{
		FrameMark;

		EProducedFrameOp producedFrameOp = EProducedFrameOp::ExecuteFull;
		SWindowEvent e = {};

		inputBackend->PreparePollEvent();

		while ((e = inputBackend->PollEvent()).type != EWindowEvent::None)
		{
			if (e.type == EWindowEvent::Quit)
			{
				bIsRunning = K_FALSE;
				producedFrameOp = EProducedFrameOp::Kill;
				break;
			}
			else
			{
				inputBackend->ProcessEvent(e, window->GetBackendWindow());
			}
		}

		if (e.type != EWindowEvent::Quit)
		{
			ZoneScopedN("Main Job");

			_app->Update();

			_context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->Clear();
			_context->GetSubsystem<CGraphics>()->ExecutePasses();
		}
		
		{
			ZoneScopedN("Wait for Released Frame");

			_sync->WaitForReleasedFrame();
		}

		{
			ZoneScopedN("Produce Frame");

			_sync->ProduceFrame(
				producedFrameOp,
				_cmd_recorder->GetRenderCommandPack()
			);
		}
	}

	//input->DestroyWindow(&window);

	// Stop render thread
	// TODO: main thread must wait until render thread finishes job completely
	_render_thread->Stop();

	_app->Stop();
}