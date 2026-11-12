// world_render_group.hpp

#pragma once

#include <vector>
#include "object.hpp"
#include "world_render_instance.hpp"
#include "camera.hpp"
#include "render_instance_motion_type.hpp"
#include "geometry_view.hpp"
#include "material.hpp"

namespace triton
{
	class XRenderGroup : public iObject
	{
		TRITON_CLASS_NAME(XRenderGroup)
		
		SHandle _render_domain;
		ERenderInstanceMotionType _motion_type;
		SGeometryView _shared_geometry;
		SHandle _shared_material;
		types::usize _max_instance_count;
		types::usize _instance_count;
		types::usize _instance_buffer_offset;
		SObjectFrame _instances;

	public:
		explicit XRenderGroup(
			cContext* context,
			types::s32 pool_index,
			ERenderInstanceMotionType motion_type,
			const SHandle& render_domain,
			const SGeometryView& shared_geometry,
			const SHandle& shared_material,
			types::usize max_instance_count
		);

		~XRenderGroup() override;

		const SHandle& AddInstance();

		void RemoveInstance(const SHandle& instance);

		inline types::boolean IsFull() const { return _instance_count >= _max_instance_count ? types::True : types::False; }

		inline const SHandle& GetRenderDomain() const { return _render_domain; }

		inline ERenderInstanceMotionType GetMotionType() const { return _motion_type; }

		inline const SGeometryView& GetSharedGeometry() const { return _shared_geometry; }

		inline const SHandle& GetSharedMaterial() const { return _shared_material; }

		inline const SObjectFrame& GetInstances() const { return _instances; }

		inline types::usize GetInstanceCount() const { return _instance_count; }

		inline types::usize GetInstanceBufferOffset() const { return _instance_buffer_offset; }

		struct TGPULayout {};
	};
}