// world_render_group.cpp

#include "world_render_group.hpp"
#include "render_instance_pool.hpp"
#include "render_instance_static_pool.hpp"
#include "render_instance_dynamic_pool.hpp"

using namespace types;

triton::XRenderGroup::XRenderGroup(
	cContext* context,
	s32 pool_index,
	ERenderInstanceMotionType motion_type,
	const SHandle& render_domain,
	const SGeometryView& shared_geometry,
	const SHandle& shared_material,
	usize max_instance_count
) :
	iObject(context, pool_index),
	_motion_type(motion_type),
	_render_domain(render_domain),
	_shared_geometry(shared_geometry),
	_shared_material(shared_material),
	_max_instance_count(max_instance_count),
	_instance_count(0),
	_instance_buffer_offset(0)
{
	_instances = SObjectFrame();

	if (!_max_instance_count)
		return;

	CRenderInstancePool* renderInstancePool = nullptr;

	if (_motion_type == ERenderInstanceMotionType::Static)
		renderInstancePool = _context->GetPool<CRenderInstanceStaticPool>();
	else if (_motion_type == ERenderInstanceMotionType::Dynamic)
		renderInstancePool = _context->GetPool<CRenderInstanceDynamicPool>();

	_instances = *renderInstancePool->CreateFrame(_max_instance_count);

	_instance_buffer_offset = _instances.begin.index;
}

triton::XRenderGroup::~XRenderGroup()
{
	CRenderInstancePool* renderInstancePool = nullptr;

	if (_motion_type == ERenderInstanceMotionType::Static)
		renderInstancePool = _context->GetPool<CRenderInstanceStaticPool>();
	else if (_motion_type == ERenderInstanceMotionType::Dynamic)
		renderInstancePool = _context->GetPool<CRenderInstanceDynamicPool>();

	_instance_buffer_offset = 0;

	renderInstancePool->DestroyFrame(_instances);

	_instance_count = 0;
}

const triton::SHandle& triton::XRenderGroup::AddInstance()
{
	if (_instance_count >= _max_instance_count)
		return SHandle();

	CRenderInstancePool* renderInstancePool = nullptr;
	
	if (_motion_type == ERenderInstanceMotionType::Static)
		renderInstancePool = _context->GetPool<CRenderInstanceStaticPool>();
	else if (_motion_type == ERenderInstanceMotionType::Dynamic)
		renderInstancePool = _context->GetPool<CRenderInstanceDynamicPool>();

	const usize index_in_group = _instance_count;

	XRenderInstance& instance = *renderInstancePool->GetFrame(_instances, index_in_group);

	instance.~XRenderInstance();

	new (&instance) XRenderInstance(
		_context,
		instance.GetPoolIndex(),
		index_in_group,
		_motion_type,
		_shared_material,
		0,
		cTransform()
	);

	_instance_count += 1;

	SHandle handle = renderInstancePool->GetHandle(_instances.begin.index + index_in_group);

	return handle;
}

void triton::XRenderGroup::RemoveInstance(const SHandle& instance)
{
	if (!_instance_count)
		return;

	CRenderInstancePool* renderInstancePool = nullptr;

	if (_motion_type == ERenderInstanceMotionType::Static)
		renderInstancePool = _context->GetPool<CRenderInstanceStaticPool>();
	else if (_motion_type == ERenderInstanceMotionType::Dynamic)
		renderInstancePool = _context->GetPool<CRenderInstanceDynamicPool>();

	if (_instance_count > 1)
	{
		XRenderInstance& cur_instance = *renderInstancePool->Get(instance);

		const usize cur_index_in_group = cur_instance.GetIndexInGroup();
		const usize last_index_in_group = _instance_count - 1;

		XRenderInstance& last_instance = *renderInstancePool->GetFrame(_instances, last_index_in_group);

		cur_instance.~XRenderInstance();

		new (&cur_instance) XRenderInstance(
			_context,
			cur_instance.GetPoolIndex(),
			last_instance.GetIndexInGroup(),
			last_instance.GetMotionType(),
			last_instance.GetMaterial(),
			last_instance.GetSkinnedBoneBufferOffset(),
			last_instance.GetTransform()
		);
	}

	_instance_count -= 1;
}