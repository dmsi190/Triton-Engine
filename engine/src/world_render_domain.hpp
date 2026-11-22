// world_render_domain.hpp

#pragma once

#include <vector>
#include <optional>
#include "camera.hpp"
#include "object.hpp"
#include "world_render_group.hpp"

namespace triton
{
	class XRenderDomain : public iObject
	{
		TRITON_OBJECT(XRenderDomain)

		XCamera*					_camera;
		std::vector<XRenderGroup*>	_renderGroups;

	public:
		explicit XRenderDomain(
			const SMemAlloc& mem_alloc,
			cContext* context,
			XCamera* camera
		) : iObject(mem_alloc, context), _camera(camera) {}

		~XRenderDomain() override = default;

		XRenderGroup* CreateRenderGroup(
			ERenderInstanceMotionType motionType,
			const SGeometryView& shared_geometry,
			XMaterial* shared_material,
			types::usize max_instance_count
		);

		void DestroyRenderGroup(XRenderGroup* renderGroup);

		inline const std::vector<XRenderGroup*>& GetRenderGroups() const { return _renderGroups; }

		inline XCamera* GetCamera() const { return _camera; }
	};
}