// world_render_domain.cpp

#include "world_render_domain.hpp"
#include "world_render_domain_pool.hpp"
#include "world_render_group_pool.hpp"

using namespace types;

std::optional<triton::SHandle> triton::XRenderDomain::CreateRenderGroup(
	ERenderInstanceMotionType motionType,
	const SGeometryView& geometry,
	const SHandle& material,
	usize max_instance_count
)
{
	CRenderGroupPool* renderGroupPool = _context->GetPool<CRenderGroupPool>();

	SHandle renderGroup = *renderGroupPool->Create(
		motionType,
		(const SHandle&)_context->GetPool<CRenderDomainPool>()->GetHandle(_poolIndex),
		geometry,
		(const SHandle&)material,
		max_instance_count
	);

	_renderGroups.push_back(renderGroup);

	return renderGroup;
}

void triton::XRenderDomain::RemoveRenderGroup(const SHandle& renderGroup)
{
	_context->GetPool<CRenderGroupPool>()->Destroy(renderGroup);
}

void triton::XRenderDomain::QueryRenderGroups()
{
	_renderGroups.clear();

	CRenderGroupPool* renderGroupPool = _context->GetPool<CRenderGroupPool>();

	SHandle thisHandle = _context->GetPool<CRenderDomainPool>()->GetHandle(this->_poolIndex);
	SBufferView<XRenderGroup> renderGroupInstancesBufferView = renderGroupPool->GetData();

	for (usize i = 0; i < renderGroupInstancesBufferView.elementCount; ++i)
	{
		XRenderGroup& renderGroup = renderGroupInstancesBufferView.elements[i];

		if (renderGroup.GetRenderDomain() == thisHandle)
			_renderGroups.push_back(renderGroupPool->GetHandle(i));
	}
}