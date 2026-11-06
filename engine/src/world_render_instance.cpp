// world_render_instance.cpp

#include "world_render_instance.hpp"
#include "render_instance_static_pool.hpp"
#include "render_instance_dynamic_pool.hpp"

void triton::XRenderInstance::WriteToStaging()
{
    CRenderInstancePool* renderInstancePool = nullptr;

    if (_motionType == ERenderInstanceMotionType::Static)
        renderInstancePool = _context->GetPool<CRenderInstanceStaticPool>();
    else if (_motionType == ERenderInstanceMotionType::Dynamic)
        renderInstancePool = _context->GetPool<CRenderInstanceDynamicPool>();

    renderInstancePool->WriteToStaging(
        _poolIndex,
        *this
    );
 }