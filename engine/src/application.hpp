// application.hpp

#pragma once

#include <chrono>
#include "object.hpp"
#include "capabilities.hpp"
#include "math.hpp"
#include "types.hpp"

namespace triton
{
    class cContext;
    class XEngine;
    class XWindow;

    class IApplication
    {
    protected:
        cContext* _context = nullptr;
        SCapabilities _caps;
        XWindow* _window = nullptr;
        XEngine* _engine = nullptr;

    public:
        explicit IApplication(cContext* context, const SCapabilities& caps);

        virtual ~IApplication();

        void CreateWindow();

        void DestroyWindow();

        virtual void Setup() = 0;

        virtual void Update() = 0;

        virtual void Stop() = 0;

        void Run();

        inline const SCapabilities& GetCapabilities() const { return _caps; }

        inline CWindow* GetWindow() const { return _window; }

        inline XEngine* GetEngine() const { return _engine; }
    };
}