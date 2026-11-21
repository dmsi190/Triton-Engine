// application.cpp

#include <iostream>
#include "application.hpp"
#include "engine.hpp"
#include "graphics.hpp"
#include "context.hpp"
#include "font_manager.hpp"
#include "filesystem_manager.hpp"
#include "physics_manager.hpp"
#include "event_manager.hpp"
#include "thread_subsystem.hpp"
#include "window.hpp"
#include "time.hpp"
#include "creator.hpp"
#include "log.hpp"

using namespace types;

triton::IApplication::IApplication(cContext* context, const SCapabilities& caps) : _context(context), _caps(caps)
{
    CMemoryAllocator::Initialize(
        caps.memory_allocator_bin_count,
        caps.memory_allocator_bin_capacity,
        caps.memory_allocator_bin_mem_step
    );

    _engine = CCreator::Create<XEngine>(_context, this);
}

triton::IApplication::~IApplication()
{
    CCreator::Destroy<XEngine>(_engine);

    CMemoryAllocator::Release();
}

void triton::IApplication::CreateWindow()
{
    _window = CCreator::Create<CWindow>(
        _context,
        _caps.window.title,
        _caps.window.size,
        _caps.window.fullscreen,
        _caps.graphicsBackend
    );
}

void triton::IApplication::DestroyWindow()
{
    CCreator::Destroy<CWindow>(_window);
}

void triton::IApplication::Run()
{
    _engine->Run();
}