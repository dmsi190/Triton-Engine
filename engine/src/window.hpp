// window.hpp

#pragma once

#include <string>
#include "math.hpp"
#include "input_window_resource.hpp"
#include "graphics_backend_type_enum.hpp"
#include "types.hpp"

namespace triton
{
    class cContext;

    class XWindow : public iObject
    {
        TRITON_OBJECT(XWindow)

        SWindowResource _window;

    public:
        explicit XWindow(
            const SMemAlloc& mem_alloc,
            cContext* context,
            const std::string& title,
            const cVector2& size,
            types::boolean fullscreen,
            EGraphicsBackendType graphicsBackend
        );

        ~XWindow() override;

        inline const cVector2& GetSize() const { return _window.size; }

        inline SWindowResource& GetBackendWindow() { return _window; }
    };
}