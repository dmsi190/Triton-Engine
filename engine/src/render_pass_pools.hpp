// render_pass_pools.hpp

#pragma once

#include "object_pool.hpp"
#include "render_pass_culling.hpp"
#include "render_pass_geometry.hpp"
#include "render_pass_processing.hpp"

namespace triton
{
    class CRenderPassCullingPool : public CObjectPool<XRenderPassCulling>
    {
        TRITON_CLASS_NAME(CRenderPassCullingPool)

    public:
        using CObjectPool<XRenderPassCulling>::CObjectPool;
        ~CRenderPassCullingPool() override = default;

        XRenderPassCulling::TGPULayout ConvertToGpuLayout(const XRenderPassCulling& object) override
        {
            XRenderPassCulling::TGPULayout gpul;

            return gpul;
        }

        void Update() override
        {
            Upload();
        }
    };

    class CRenderPassGeometryPool : public CObjectPool<XRenderPassGeometry>
    {
        TRITON_CLASS_NAME(CRenderPassGeometryPool)

    public:
        using CObjectPool<XRenderPassGeometry>::CObjectPool;
        ~CRenderPassGeometryPool() override = default;

        XRenderPassGeometry::TGPULayout ConvertToGpuLayout(const XRenderPassGeometry& object) override
        {
            XRenderPassGeometry::TGPULayout gpul;

            return gpul;
        }

        void Update() override
        {
            Upload();
        }
    };

    class CRenderPassProcessingPool : public CObjectPool<XRenderPassProcessing>
    {
        TRITON_CLASS_NAME(CRenderPassProcessingPool)

    public:
        using CObjectPool<XRenderPassProcessing>::CObjectPool;
        ~CRenderPassProcessingPool() override = default;

        XRenderPassProcessing::TGPULayout ConvertToGpuLayout(const XRenderPassProcessing& object) override
        {
            XRenderPassProcessing::TGPULayout gpul;

            return gpul;
        }

        void Update() override
        {
            Upload();
        }
    };
}