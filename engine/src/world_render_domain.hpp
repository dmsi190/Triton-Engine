// world_render_domain.hpp

#pragma once

#include <vector>
#include <optional>
#include "camera.hpp"
#include "object.hpp"
#include "world_render_group.hpp"
#include "handle.hpp"

namespace triton
{
	class XRenderDomain : public iObject
	{
		TRITON_CLASS_NAME(XRenderDomain)

		SHandle					_camera;
		std::vector<SHandle>	_renderGroups;

	public:
		explicit XRenderDomain(cContext* context, types::s32 poolIndex) : iObject(context, poolIndex) {}

		explicit XRenderDomain(
			cContext* context,
			types::s32 poolIndex,
			const SHandle& camera
		) : iObject(context, poolIndex), _camera(camera) {}

		~XRenderDomain() override = default;

		std::optional<SHandle> CreateRenderGroup(
			ERenderInstanceMotionType motionType,
			const SGeometryView& geometry,
			const SHandle& material,
			types::usize max_instance_count
		);

		void RemoveRenderGroup(const SHandle& renderGroup);

		void QueryRenderGroups();

		inline const std::vector<SHandle>& GetRenderGroups() const { return _renderGroups; }

		inline const SHandle& GetCamera() const { return _camera; }

		struct TGPULayout {};
	};
}