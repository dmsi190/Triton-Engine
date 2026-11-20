// render_thread.hpp

#pragma once

#include <cstring>
#include <atomic>
#include <cassert>
#include <thread>
#include "thread_subsystem.hpp"
#include "synchronization.hpp"
#include "types.hpp"

namespace triton
{
	class cContext;
	class IGraphicsBackend2;
	class CGraphics;
	class XSynchronization;
	class CWindow;
	struct SRenderCommandPack;

	class XRenderThread final : public XThread
	{
		TRITON_OBJECT(XRenderThread)

		XSynchronization*	_sync;
		CWindow*			_window;

	public:
		explicit XRenderThread(
			const SMemAlloc& mem_alloc,
			cContext* context,
			XSynchronization* sync,
			CWindow* window
		) : XThread(mem_alloc, context), _sync(sync), _window(window) {}

		~XRenderThread() override = default;

		virtual void ThreadFunction() override;

	private:
		void ExecuteCommands(
			const SRenderCommandPack& renderCommandPack,
			IGraphicsBackend2* gfxBackend
		);

		void Present(IGraphicsBackend2* gfxBackend);
	};
}
