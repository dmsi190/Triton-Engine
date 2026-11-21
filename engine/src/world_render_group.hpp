// world_render_group.hpp

#pragma once

#include <vector>
#include "object.hpp"
#include "world_render_instance.hpp"
#include "camera.hpp"
#include "render_instance_motion_type.hpp"
#include "geometry_view.hpp"
#include "material.hpp"
#include "gpu_buffer.hpp"

namespace triton
{
	class XRenderDomain;

	struct SRenderGroupGPULayout final
	{
		types::s32			camera_index;
		types::dword		motion_type;
		types::u32			instance_buffer_offset;
		SGeometryDrawInfo	geometry_draw_info;
	};

	class XRenderGroup : public iObject
	{
		TRITON_OBJECT(XRenderGroup)
		
		XRenderDomain*					_render_domain;
		ERenderInstanceMotionType		_motion_type;
		SGeometryView					_shared_geometry;
		types::usize					_max_instance_count;
		types::usize					_instance_count;
		types::usize					_instance_buffer_start_offset;
		SGPUBufferFrame					_instance_buffer_frame;
		SMemAlloc						_instances_memory;
		SRenderInstance*				_instances;
		SMemAlloc						_free_list_memory;
		types::usize*					_free_list;
		types::usize					_free_list_capacity;
		types::s32						_head;
		types::s32						_tail;

	public:
		explicit XRenderGroup(
			const SMemAlloc& mem_alloc,
			cContext* context,
			ERenderInstanceMotionType motion_type,
			XRenderDomain* render_domain,
			const SGeometryView& shared_geometry,
			types::usize max_instance_count
		);

		~XRenderGroup() override;

		const SRenderInstance* AddInstance(XWorldObject* connected_object);

		void RemoveInstance(SRenderInstance* instance);

		inline XRenderDomain* GetRenderDomain() const { return _render_domain; }

		inline ERenderInstanceMotionType GetMotionType() const { return _motion_type; }

		inline const SGeometryView& GetSharedGeometry() const { return _shared_geometry; }

		inline types::usize GetInstanceCount() const { return _instance_count; }

		inline types::usize GetInstanceBufferStartOffset() const { return _instance_buffer_start_offset; }
	};
}