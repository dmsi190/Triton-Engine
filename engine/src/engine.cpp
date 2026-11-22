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
#include "model_backend.hpp"
#include "model_backend_assimp.hpp"
#include "synchronization.hpp"
#include "render_thread.hpp"
#include "window.hpp"
#include "image_decoder_multi.hpp"
#include "graphics_backend2_vulkan.hpp"
#include "world.hpp"
#include "render_command_proxy.hpp"
#include "graphics_buffers.hpp"
#include "creator.hpp"
#include "render_thread.hpp"

using namespace types;

triton::XEngine::XEngine(const SMemAlloc& mem_alloc, cContext* context, IApplication* app)
	: XSubsystem(mem_alloc, context), _app(app)
{
	Initialize();
}

triton::XEngine::~XEngine()
{
	Shutdown();
}

void triton::XEngine::Initialize()
{
	CThreadGuard::CaptureMainThreadId();

	const SCapabilities& caps = _app->GetCapabilities();

	// NOTE: The order of the objects registered below matters

	// Register Engine as subsystem
	_context->RegisterSubsystem(this);

	// Set context for render command proxy
	CRenderCommandProxy::SetContext(_context);

	// Register backends
	_context->RegisterBackend<IInputBackend>(CCreator::Create<XInputBackendSDL>(_context));
	//_context->RegisterBackend<iAudioBackend>(new cAudioBackendOAL(_context));
	_context->RegisterBackend<IModelBackend>(CCreator::Create<XModelBackendAssimp>(_context));
	_context->RegisterBackend<IImageDecoder>(CCreator::Create<BImageDecoderMulti>(_context));

	// Create application window
	_app->CreateWindow();

	// Register file system subsystem before graphics backend
	_context->RegisterSubsystem(CCreator::Create<XFileSystem>(_context));

	// Register graphics backend
	_context->RegisterBackend<IGraphicsBackend2>(CCreator::Create<BGraphicsBackend2Vulkan>(_context));

	// Initialize render thread
	InitializeRenderCommandRecorder();
	InitializeSynchronization();
	InitializeRenderThread();

	// Initialize graphics buffers
	CGraphicsBuffers::Initialize(_context);

	// Register subsystems
	_context->RegisterSubsystem(CCreator::Create<XGeometryStorage>(_context));

	_context->RegisterSubsystem(CCreator::Create<XGraphics>(_context));
}

void triton::XEngine::Shutdown()
{
	CGraphicsBuffers::Release();
	ReleaseRenderThread();
	ReleaseSynchronization();
	ReleaseRenderCommandRecorder();
	_app->DestroyWindow();
}

void triton::XEngine::Run()
{
	MainThreadFunction();
}

void triton::XEngine::InitializeSynchronization()
{
	_sync = CCreator::Create<XSynchronization>(_context, _cmd_recorder, 1024);
}

void triton::XEngine::InitializeRenderCommandRecorder()
{
	CThreadGuard::AssertMain();

	_cmd_recorder = CCreator::Create<XRenderCommandRecorder>(_context);
}

void triton::XEngine::InitializeRenderThread()
{
	CThreadGuard::AssertMain();

	// Create render thread
	_render_thread = CCreator::Create<XRenderThread>(_context, _sync, _app->GetWindow());
	_render_thread->Run();

	// Wait until render thread gets initialized
	_sync->WaitForRenderThreadToInit();
}

void triton::XEngine::ReleaseSynchronization()
{
	CCreator::Destroy<XSynchronization>(_sync);
}

void triton::XEngine::ReleaseRenderCommandRecorder()
{
	CCreator::Destroy<XRenderCommandRecorder>(_cmd_recorder);
}

void triton::XEngine::ReleaseRenderThread()
{
	_render_thread->Stop();

	CCreator::Destroy<XRenderThread>(_render_thread);
}

void triton::XEngine::MainThreadFunction()
{
	CThreadGuard::AssertMain();

	std::cout << "Main thread started\n";

	tracy::SetThreadName("Main Thread");

	if (!_app)
		return;

	_app->Setup();

	boolean bIsRunning = True;

	IInputBackend* inputBackend = _context->GetBackend<IInputBackend>();
	XWindow* window = _app->GetWindow();

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

			_context->GetSubsystem<XEngine>()->GetRenderCommandRecorder()->Clear();
			_context->GetSubsystem<XGraphics>()->ExecutePasses();
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