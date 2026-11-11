// world_object.cpp

#include "world_object.hpp"
#include "world_render_group_pool.hpp"
#include "world_render_group_instance_pool.hpp"
#include "render_instance_static_pool.hpp"
#include "render_instance_dynamic_pool.hpp"
#include "animation_pool.hpp"

using namespace types;

triton::XWorldObject::XWorldObject(
	cContext* context,
	types::s32 poolIndex,
	ERenderInstanceMotionType motionType
) : iObject(context, poolIndex), _motion_type(motionType) {}

void triton::XWorldObject::SetWorldPosition(const cVector3& world_position)
{
	CRenderInstancePool* render_instance_pool = nullptr;

	if (_motion_type == ERenderInstanceMotionType::Static)
		render_instance_pool = _context->GetPool<CRenderInstanceStaticPool>();
	else if (_motion_type == ERenderInstanceMotionType::Dynamic)
		render_instance_pool = _context->GetPool<CRenderInstanceDynamicPool>();

	auto render_instance_lookup = render_instance_pool->Get(_renderInstance);

	if (!render_instance_lookup.has_value())
		return;

	XRenderInstance& render_instance = *render_instance_lookup;

	render_instance.SetWorldPosition(world_position);
	render_instance.Transform();
}

void triton::XWorldObject::SetRenderable(
	types::boolean bIsRenderable,
	const SHandle& renderDomain,
	const SGeometryView& geometry,
	const XMaterial::THandle& material
)
{
	if (bIsRenderable == False)
	{
		auto renderGroupInstanceLookup = _context->GetPool<CRenderGroupInstancePool>()->Get(_renderGroupInstance);

		if (!renderGroupInstanceLookup.has_value())
			return;

		XRenderGroupInstance& renderGroupInstance = *renderGroupInstanceLookup;
		renderGroupInstance.RemoveInstance(_renderInstance);

		_renderDomain = XRenderDomain::THandle();
		_renderGroupInstance = XRenderGroupInstance::THandle();
		_renderInstance = XRenderInstance::THandle();
	}
	else
	{
		auto renderGroupInstanceLookup = FindRenderGroupInstance(
			renderDomain,
			_motion_type,
			geometry,
			material
		);

		if (!renderGroupInstanceLookup.has_value())
		{
			CRenderGroupPool* renderGroupPool = _context->GetPool<CRenderGroupPool>();

			auto renderGroupLookup = renderGroupPool->Create(
				renderDomain,
				_motion_type,
				geometry,
				material
			);

			XRenderGroup& renderGroup = *renderGroupPool->Get(*renderGroupLookup);
			_renderGroupInstance = renderGroup.CreateInstance(1024);

			XRenderGroupInstance& renderGroupInstance =
				*_context->GetPool<CRenderGroupInstancePool>()->Get(_renderGroupInstance);

			_renderInstance = renderGroupInstance.AddInstance();

			_renderDomain = renderDomain;
		}
		else
		{
			XRenderGroupInstance& renderGroupInstance =
				*_context->GetPool<CRenderGroupInstancePool>()->Get(*renderGroupInstanceLookup);

			_renderInstance = renderGroupInstance.AddInstance();

			_renderDomain = renderDomain;
			_renderGroupInstance = *renderGroupInstanceLookup;
		}
	}
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

std::optional<triton::XRenderGroupInstance::THandle> triton::XWorldObject::FindRenderGroupInstance(
	const SHandle& renderDomain,
	ERenderInstanceMotionType motionType,
	const SGeometryView& geometry,
	const XMaterial::THandle& material
)
{
	auto renderGroupInstancePool = _context->GetPool<CRenderGroupInstancePool>();

	SBufferView<XRenderGroupInstance> renderGroupInstancesBufferView = renderGroupInstancePool->GetData();

	for (usize i = 0; i < renderGroupInstancesBufferView.elementCount; ++i)
	{
		XRenderGroupInstance& renderGroupInstance = renderGroupInstancesBufferView.elements[i];

		if (IsRenderGroupInstanceFound(
			renderGroupInstance,
			renderDomain,
			motionType,
			geometry,
			material
		) == True)
			return renderGroupInstancePool->GetHandle(i);
	}

	return std::nullopt;
}

types::boolean triton::XWorldObject::IsRenderGroupInstanceFound(
	const XRenderGroupInstance& compareRenderGroupInstance,
	const SHandle& renderDomain,
	ERenderInstanceMotionType motionType,
	const SGeometryView& geometry,
	const XMaterial::THandle& material
)
{
	const SHandle& compareRenderGroupInstanceDomain = compareRenderGroupInstance.GetRenderDomain();
	ERenderInstanceMotionType compareRenderGroupInstanceMotionType = compareRenderGroupInstance.GetMotionType();
	const SGeometryView& compareRenderGroupInstanceGeometry = compareRenderGroupInstance.GetSharedGeometry();
	const XMaterial::THandle& compareRenderGroupInstanceMaterial = compareRenderGroupInstance.GetSharedMaterial();

	if (compareRenderGroupInstanceDomain == renderDomain &&
		compareRenderGroupInstanceMotionType == motionType &&
		compareRenderGroupInstanceGeometry._format == geometry._format &&
		compareRenderGroupInstanceGeometry._indexCount == geometry._indexCount &&
		compareRenderGroupInstanceGeometry._indexData == geometry._indexData &&
		compareRenderGroupInstanceGeometry._indexElementOffset == geometry._indexElementOffset &&
		compareRenderGroupInstanceGeometry._vertexCount == geometry._vertexCount &&
		compareRenderGroupInstanceGeometry._vertexData == geometry._vertexData &&
		compareRenderGroupInstanceGeometry._vertexElementOffset == geometry._vertexElementOffset &&
		compareRenderGroupInstanceMaterial == material &&
		compareRenderGroupInstance.IsFull() == False)
		return True;

	return False;
}