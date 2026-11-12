// world_object.cpp

#include "world_object.hpp"
#include "world_render_group_pool.hpp"
#include "render_instance_static_pool.hpp"
#include "render_instance_dynamic_pool.hpp"
#include "animation_pool.hpp"

using namespace types;

triton::XWorldObject::XWorldObject(
	cContext* context,
	s32 pool_index,
	const SHandle& render_group_handle,
	const cVector3& world_position
) :
	iObject(context, pool_index),
	_render_group(render_group_handle)
{
	XRenderGroup& render_group = *_context->GetPool<CRenderGroupPool>()->Get(render_group_handle);

	_render_instance = render_group.AddInstance();

	_motion_type = render_group.GetMotionType();

	SetWorldPosition(world_position);
}

void triton::XWorldObject::SetWorldPosition(const cVector3& world_position)
{
	CRenderInstancePool* render_instance_pool = nullptr;

	if (_motion_type == ERenderInstanceMotionType::Static)
		render_instance_pool = _context->GetPool<CRenderInstanceStaticPool>();
	else if (_motion_type == ERenderInstanceMotionType::Dynamic)
		render_instance_pool = _context->GetPool<CRenderInstanceDynamicPool>();

	auto render_instance_lookup = render_instance_pool->Get(_render_instance);

	if (!render_instance_lookup.has_value())
		return;

	XRenderInstance& render_instance = *render_instance_lookup;

	render_instance.SetWorldPosition(world_position);
	render_instance.Transform();
}

void triton::XWorldObject::PlayAnimation(types::usize index)
{
	XAnimation& animation = *_context->GetPool<CAnimationPool>()->Get(_animations[index]);
	XSkin& skin = *_context->GetPool<CSkinPool>()->Get(_skin);

	static f32 time = 0.0f;

	SAnimationFrame frame = *animation.EvaluateFrame(_skeleton, time);
	skin.Skin(frame);

	time += 0.1f;
}