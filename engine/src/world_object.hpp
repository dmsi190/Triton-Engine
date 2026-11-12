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
		TRITON_CLASS_NAME(XWorldObject)

		ERenderInstanceMotionType			_motion_type = ERenderInstanceMotionType::Unknown;
		SHandle								_render_group;
		SHandle								_render_instance;
		SHandle								_skeleton;
		SHandle								_skin;
		std::vector<SHandle>				_animations = {};

	public:
		explicit XWorldObject(
			cContext* context,
			types::s32 pool_index,
			const SHandle& render_group_handle,
			const cVector3& world_position = cVector3(0.0f)
		);

		~XWorldObject() override = default;

		void SetWorldPosition(const cVector3& world_position);

		void PlayAnimation(types::usize index);

		struct TGPULayout {};
	};
}