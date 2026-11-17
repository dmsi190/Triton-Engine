// event_types.hpp

#include <functional>

namespace triton
{
    class iObject;
    class cContext;

    using EventFunction = std::function<void(iObject* self, cContext* context, void* data)>;

    enum class eEventType
    {
        Unknown,

        KeyPress,

        FrameBegin,
        FrameUpdate,
        FrameEnd
    };
}