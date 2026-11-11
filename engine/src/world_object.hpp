// world_object.hpp

#pragma once

#include "object.hpp"
#include "world_render_domain.hpp"
#include "world_render_group_instance.hpp"
#include "skeleton.hpp"
#include "skin.hpp"
#include "animation.hpp"

namespace triton
{
	class XWorldObject : public iObject
	{
		TRITON_CLASS_NAME(XWorldObject)

		ERenderInstanceMotionType			_motion_type = ERenderInstanceMotionType::Unknown;
		SHandle								_renderDomain;
		XRenderGroupInstance::THandle		_renderGroupInstance;
		XRenderInstance::THandle			_renderInstance;
		XSkeleton::THandle                  _skeleton;
		XSkin::THandle                      _skin;
		std::vector<XAnimation::THandle>    _animations = {};

	public:
		explicit XWorldObject(
			cContext* context,
			types::s32 poolIndex,
			ERenderInstanceMotionType motionType
		);

		~XWorldObject() override = default;

		void SetWorldPosition(const cVector3& world_position);

		void SetRenderable(
			types::boolean bIsRenderable,
			const SHandle& renderDomain = SHandle(),
			const SGeometryView& geometry = SGeometryView(),
			const XMaterial::THandle& material = XMaterial::THandle()
		);

		void PlayAnimation(types::usize index);

		struct THandle : public SHandle {};

		struct TGPULayout {};

	private:
		std::optional<XRenderGroupInstance::THandle> FindRenderGroupInstance(
			const SHandle& renderDomain,
			ERenderInstanceMotionType motionType,
			const SGeometryView& geometry,
			const XMaterial::THandle& material
		);

		types::boolean IsRenderGroupInstanceFound(
			const XRenderGroupInstance& compareRenderGroupInstance,
			const SHandle& renderDomain,
			ERenderInstanceMotionType motionType,
			const SGeometryView& geometry,
			const XMaterial::THandle& material
		);
	};
}