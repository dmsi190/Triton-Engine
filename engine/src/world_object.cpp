// world_object.cpp

#include "world_object.hpp"

using namespace types;

triton::XWorldObject::XWorldObject(
	const SMemAlloc& mem_alloc,
	cContext* context,
	XRenderGroup* render_group,
	const cVector3& world_position
) : iObject(mem_alloc, context),
	_render_group(render_group)
{
	_render_instance = _render_group->AddInstance(this);

	_motion_type = _render_group->GetMotionType();

	SetWorldPosition(world_position);
}

void triton::XWorldObject::SetWorldPosition(const cVector3& world_position)
{
}

void triton::XWorldObject::PlayAnimation(types::usize index)
{
	//XAnimation& animation = *_context->GetPool<CAnimationPool>()->Get(_animations[index]);
	//XSkin& skin = *_context->GetPool<CSkinPool>()->Get(_skin);

	//static f32 time = 0.0f;

	//SAnimationFrame frame = *animation.EvaluateFrame(_skeleton, time);
	//skin.Skin(frame);

	//time += 0.1f;
}