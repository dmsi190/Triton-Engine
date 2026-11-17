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
	class cRenderThread;

	inline std::thread::id global_main_thread_id = std::this_thread::get_id();

	class CEngine final : public CSubsystem
	{
		TRITON_CLASS_NAME(CEngine)

		IApplication* _app = nullptr;
		XSynchronization* _sync = nullptr;
		XRenderCommandRecorder* _cmd_recorder = nullptr;
		cRenderThread* _render_thread = nullptr;

	public:
		explicit CEngine(cContext* context, IApplication* app);

		~CEngine() override;

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