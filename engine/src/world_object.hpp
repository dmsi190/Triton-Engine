// world_object.hpp

#pragma once

#include "object.hpp"
#include "world_render_domain.hpp"
#include "world_render_group.hpp"
#include "skeleton.hpp"
#include "skin.hpp"
#include "animation.hpp"

namespace triton
{
	class XWorldObject : public iObject
	{
		TRITON_OBJECT(XWorldObject)

		ERenderInstanceMotionType			_motion_type = ERenderInstanceMotionType::Unknown;
		XRenderGroup*						_render_group;
		const SRenderInstance*				_render_instance;
		cTransform							_transform;
		XMaterial*							_material;
		XSkeleton*							_skeleton;
		XSkin*								_skin;
		std::vector<XAnimation*>			_animations = {};

	public:
		explicit XWorldObject(
			const SMemAlloc& mem_alloc,
			cContext* context,
			XRenderGroup* render_group,
			const cVector3& world_position = cVector3(0.0f)
		);

		~XWorldObject() override = default;

		void SetWorldPosition(const cVector3& world_position);

		void PlayAnimation(types::usize index);

		inline ERenderInstanceMotionType GetMotionType() const
		{
			return _motion_type;
		}

		inline types::usize GetMaterialIndex() const
		{
			return 0;
		}

		inline const cMatrix4& GetWorldMatrix() const
		{
			return _transform.GetWorld();
		}
	};
}