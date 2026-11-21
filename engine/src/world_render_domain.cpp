// world_render_domain.cpp

#include "world_render_domain.hpp"
#include "creator.hpp"

using namespace types;

triton::XRenderGroup* triton::XRenderDomain::CreateRenderGroup(
	ERenderInstanceMotionType motionType,
	const SGeometryView& shared_geometry,
	usize max_instance_count
)
{
	XRenderGroup* render_group = CCreator::Create<XRenderGroup>(
		_context,
		motionType,
		this,
		shared_geometry,
		max_instance_count
	);

	_renderGroups.push_back(render_group);

	return render_group;
}

void triton::XRenderDomain::DestroyRenderGroup(XRenderGroup* renderGroup)
{
	CCreator::Destroy<XRenderGroup>(renderGroup);
}