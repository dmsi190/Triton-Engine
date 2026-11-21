// engine.hpp

#pragma once

#include <thread>
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include "object.hpp"
#include "subsystem.hpp"
#include "types.hpp"

namespace triton
{
	class cContext;
	class IApplication;
	class XSynchronization;
	class XRenderCommandRecorder;
	class XRenderThread;

	inline std::thread::id global_main_thread_id = std::this_thread::get_id();

	class XEngine final : public XSubsystem
	{
		TRITON_OBJECT(XEngine)

		IApplication*				_app;
		XSynchronization*			_sync;
		XRenderCommandRecorder*		_cmd_recorder;
		XRenderThread*				_render_thread;

	public:
		explicit XEngine(const SMemAlloc& mem_alloc, cContext* context, IApplication* app);

		~XEngine() override;

		void Initialize();
		void Shutdown();
		void Run();

		inline IApplication* GetApplication() const { return _app; }

		inline XRenderCommandRecorder* GetRenderCommandRecorder() { return _cmd_recorder; }

		inline XSynchronization* GetSynchronization() { return _sync; }

	private:
		void InitializeSynchronization();
		void InitializeRenderCommandRecorder();
		void InitializeRenderThread();
		void ReleaseSynchronization();
		void ReleaseRenderCommandRecorder();
		void ReleaseRenderThread();
		void MainThreadFunction();
	};
}