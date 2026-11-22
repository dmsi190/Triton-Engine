// window.cpp

#include "window.hpp"
#include "context.hpp"
#include "input_backend.hpp"

using namespace types;

triton::XWindow::XWindow(
	const SMemAlloc& mem_alloc,
	cContext* context,
	const std::string& title,
	const cVector2& size,
	boolean fullscreen,
	EGraphicsBackendType graphicsBackend
) : iObject(mem_alloc, context)
{
	_window = _context->GetBackend<IInputBackend>()->CreateBackendWindow(
		title,
		size,
		fullscreen,
		graphicsBackend
	);
}

triton::XWindow::~XWindow()
{
	_context->GetBackend<IInputBackend>()->DestroyBackendWindow(_window);
}