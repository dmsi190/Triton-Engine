// world_object.cpp

#include "world_object.hpp"
#include "world_render_group_pool.hpp"

using namespace types;

triton::XWorldObject::XWorldObject(
	const SMemAlloc& mem_alloc,
	cContext* context,
	XRenderGroup* render_group,
	const cVector3& world_position = cVector3(0.0f)
) : iObject(mem_alloc, context),
	_render_group(render_group)
{
	_render_instance = _render_group->AddInstance();

	_motion_type = _render_group->GetMotionType();

	SetWorldPosition(world_position);
}

void triton::XWorldObject::SetWorldPosition(const cVector3& world_position)
{
	_render_instance->SetWorldPosition(world_position);
	_render_instance->Transform();
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