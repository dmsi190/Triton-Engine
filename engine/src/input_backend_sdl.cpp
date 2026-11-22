// input_backend_sdl.hpp

#pragma once

#include <cstring>
#include <SDL3/SDL_vulkan.h>
#include "input_backend_sdl.hpp"
#include "context.hpp"
#include "types.hpp"

using namespace types;

triton::XInputBackendSDL::XInputBackendSDL(const SMemAlloc& mem_alloc, cContext* context)
    : IInputBackend(mem_alloc, context)
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        Print("Error: SDL_Init failed");
        return;
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#ifdef _DEBUG
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    InitializeKeyMap();
}

triton::SWindowResource triton::XInputBackendSDL::CreateBackendWindow(
    const std::string& title,
    const cVector2& size,
    boolean fullscreen,
    EGraphicsBackendType graphicsBackend
)
{
    SWindowResource wb = {};
    wb.title = title;
    wb.size = size;
    wb.fullscreen = fullscreen;

    if (fullscreen == False)
    {
        u64 backendIdentifier = 0;
        if (graphicsBackend == EGraphicsBackendType::OpenGL)
            backendIdentifier = SDL_WINDOW_OPENGL;
        else if (graphicsBackend == EGraphicsBackendType::Vulkan)
            backendIdentifier = SDL_WINDOW_VULKAN;

        wb.instance = (qword)SDL_CreateWindow(
            wb.title.c_str(),
            wb.size.GetX(),
            wb.size.GetY(),
            backendIdentifier | SDL_WINDOW_RESIZABLE
        );

        SDL_SetWindowRelativeMouseMode((SDL_Window*)wb.instance, true);
    }
    else
    {
        // TODO: add fullscreen mode
    }

    if (!wb.instance)
    {
        Print("Error: SDL_CreateWindow failed");
        return wb;
    }

    return wb;
}

void triton::XInputBackendSDL::DestroyBackendWindow(SWindowResource& window)
{
    if (window.instance == 0)
        return;

    SDL_DestroyWindow((SDL_Window*)window.instance);
}

void triton::XInputBackendSDL::ResizeWindow(SWindowResource& window, const cVector2& newSize)
{
    if (window.instance == 0)
        return;

    window.size = cVector2(newSize.GetX(), newSize.GetY());
}

std::vector<const char*> triton::XInputBackendSDL::GetBackendWindowVulkanExtensions()
{
    uint32_t extensionCount = 0;
    const char* const* extensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);

    std::vector<const char*> extensionsVector(extensionCount);
    for (usize i = 0; i < extensionCount; i++)
        extensionsVector.push_back(extensions[i]);

    return extensionsVector;
}

void* triton::XInputBackendSDL::CreateBackendWindowVulkanSurface(SWindowResource& window, void* instance)
{
    VkInstance sdlInstance = {};
    std::memcpy(&sdlInstance, instance, sizeof(VkInstance));

    VkSurfaceKHR sdlSurface = {};

    SDL_Vulkan_CreateSurface(
        (SDL_Window*)window.instance,
        sdlInstance,
        nullptr,
        &sdlSurface
    );

    VkSurfaceKHR* heapSurface = new VkSurfaceKHR;
    std::memcpy(heapSurface, &sdlSurface, sizeof(VkSurfaceKHR));

    return heapSurface;
}

void triton::XInputBackendSDL::DestroyBackendWindowVulkanSurface(void* instance, void* surface)
{
    VkInstance sdlInstance = {};
    std::memcpy(&sdlInstance, instance, sizeof(VkInstance));

    VkSurfaceKHR sdlSurface = {};
    std::memcpy(&sdlSurface, surface, sizeof(VkSurfaceKHR));

    SDL_Vulkan_DestroySurface(
        sdlInstance,
        sdlSurface,
        nullptr
    );

    delete surface;
}

void triton::XInputBackendSDL::PreparePollEvent()
{
    _bMouseMoved = False;
    _mouseDelta = cVector2(0.0f);
}

triton::SWindowEvent triton::XInputBackendSDL::PollEvent()
{
    SWindowEvent e = {};
    e.type = EWindowEvent::None;

    SDL_Event event = {};

    if (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_EVENT_KEY_DOWN:
            {
                e.type = EWindowEvent::KeyDown;
                e.argA = event.key.scancode;
                return e;
            }
            case SDL_EVENT_KEY_UP:
            {
                e.type = EWindowEvent::KeyUp;
                e.argA = event.key.scancode;
                return e;
            }
            case SDL_EVENT_WINDOW_FOCUS_GAINED:
            {
                e.type = EWindowEvent::FocusGained;
                return e;
            }
            case SDL_EVENT_WINDOW_FOCUS_LOST:
            {
                e.type = EWindowEvent::FocusLost;
                return e;
            }
            case SDL_EVENT_WINDOW_RESIZED:
            {
                e.type = EWindowEvent::Resized;
                e.argA = event.window.data1;
                e.argB = event.window.data2;
                return e;
            }
            case SDL_EVENT_MOUSE_MOTION:
            {
                _bMouseMoved = True;

                e.type = EWindowEvent::MouseMotion;
                e.argA = event.motion.x;
                e.argB = event.motion.y;
                e.argC = event.motion.xrel;
                e.argD = event.motion.yrel;
                return e;
            }
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            {
                e.type = EWindowEvent::MouseButtonDown;
                e.argA = event.button.button;
                return e;
            }
            case SDL_EVENT_MOUSE_BUTTON_UP:
            {
                e.type = EWindowEvent::MouseButtonUp;
                e.argA = event.button.button;
                return e;
            }
            case SDL_EVENT_QUIT:
            {
                e.type = EWindowEvent::Quit;
                return e;
            }
        }
    }

    return e;
}

void triton::XInputBackendSDL::ProcessEvent(const SWindowEvent& event, SWindowResource& window)
{
    switch (event.type)
    {
        case EWindowEvent::KeyDown:
        {
            SetKey(event.argA, EKeyState::Pressed);
            break;
        }

        case EWindowEvent::KeyUp:
        {
            SetKey(event.argA, EKeyState::Released);
            break;
        }

        case EWindowEvent::FocusGained:
        {
            if (IsWindowFocused() == K_FALSE)
                SetWindowFocus(K_TRUE);
            break;
        }

        case EWindowEvent::FocusLost:
        {
            SetWindowFocus(K_FALSE);
            break;
        }

        case EWindowEvent::Resized:
        {
            ResizeWindow(window, cVector2(event.argA, event.argB));
            break;
        }

        case EWindowEvent::MouseMotion:
        {
            SetWindowMouse(cVector2(event.argA, event.argB), cVector2(event.argC, event.argD));
            break;
        }

        case EWindowEvent::MouseButtonDown:
        {
            SetKey(event.argA, EKeyState::Pressed);
            break;
        }

        case EWindowEvent::MouseButtonUp:
        {
            SetKey(event.argB, EKeyState::Released);
            break;
        }
    }
}

void* triton::XInputBackendSDL::GetWindowWin32Handle(SWindowResource& window)
{
    if (window.instance == 0)
        return nullptr;

    return nullptr; //(void*)glfwGetWin32Window((GLFWwindow*)window.instance);
}

triton::EKeyState triton::XInputBackendSDL::GetKey(EKeyCode key)
{
    return _keyState[(usize)key];
}

triton::cVector2 triton::XInputBackendSDL::GetCursorPosition(SWindowResource& window)
{
    return _cursorPosition;
}

triton::cVector2 triton::XInputBackendSDL::GetMouseDelta()
{
    if (_bMouseMoved == True)
        return _mouseDelta;
    else
        return cVector2(0.0f);
}

triton::cVector2 triton::XInputBackendSDL::GetMonitorSize()
{
    return cVector2(0.0f); //cVector2(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
}

void triton::XInputBackendSDL::SetKey(types::cpuword internalKey, EKeyState state)
{
    auto result = _keyMap.find(internalKey);
    if (result != _keyMap.end())
        _keyState[(usize)result->second] = state;
}

void triton::XInputBackendSDL::SetWindowFocus(boolean isFocused)
{
    _isFocused = isFocused;
}

void triton::XInputBackendSDL::SetWindowMouse(const cVector2& cursorPosition, const cVector2& mouseDelta)
{
    _cursorPosition = cursorPosition;
    _mouseDelta.AddX(mouseDelta.GetX());
    _mouseDelta.AddY(mouseDelta.GetY());
}

void triton::XInputBackendSDL::SetVSync(cpuword flag)
{
    // TODO: Implement SDL VSync
}

boolean triton::XInputBackendSDL::IsWindowFocused()
{
    return _isFocused;
}

void triton::XInputBackendSDL::InitializeKeyMap()
{
    _keyMap.emplace(SDL_BUTTON_LEFT, EKeyCode::MouseLeft);
    _keyMap.emplace(SDL_BUTTON_MIDDLE, EKeyCode::MouseMiddle);
    _keyMap.emplace(SDL_BUTTON_RIGHT, EKeyCode::MouseRight);
    _keyMap.emplace(SDL_SCANCODE_W, EKeyCode::W);
    _keyMap.emplace(SDL_SCANCODE_A, EKeyCode::A);
    _keyMap.emplace(SDL_SCANCODE_S, EKeyCode::S);
    _keyMap.emplace(SDL_SCANCODE_D, EKeyCode::D);
}