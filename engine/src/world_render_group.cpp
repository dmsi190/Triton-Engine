// world_render_group.cpp

#include "world_render_group.hpp"
#include "graphics_buffers.hpp"

using namespace types;

triton::XRenderGroup::XRenderGroup(
	const SMemAlloc& mem_alloc,
	cContext* context,
	ERenderInstanceMotionType motion_type,
	XRenderDomain* render_domain,
	const SGeometryView& shared_geometry,
	types::usize max_instance_count
) : iObject(mem_alloc, context),
	_motion_type(motion_type),
	_render_domain(render_domain),
	_shared_geometry(shared_geometry),
	_max_instance_count(max_instance_count),
	_instance_count(0),
	_instance_buffer_start_offset(0)
{
	if (!_max_instance_count)
		return;

	if (_motion_type == ERenderInstanceMotionType::Static)
		_instance_buffer_frame =
			CGraphicsBuffers::GetStaticInstanceBuffer()->AcquireFreeStagingFrame(_max_instance_count);
	else if (_motion_type == ERenderInstanceMotionType::Dynamic)
		_instance_buffer_frame =
			CGraphicsBuffers::GetDynamicInstanceBuffer()->AcquireFreeStagingFrame(_max_instance_count);

	_instance_buffer_start_offset = _instance_buffer_frame.begin_index;

	_instances_memory = CMemoryAllocator::Allocate(_max_instance_count * sizeof(SRenderInstance));
	_instances = (SRenderInstance*)_instances_memory.mem;

	_free_list_memory = CMemoryAllocator::Allocate(_max_instance_count * sizeof(usize));
	_free_list = (usize*)_free_list_memory.mem;
	_free_list_capacity = 0;

	for (usize i = 0; i < _max_instance_count; ++i, ++_free_list_capacity)
		_free_list[i] = i;

	_head = -1;
	_tail = -1;
}

triton::XRenderGroup::~XRenderGroup()
{
	CMemoryAllocator::Deallocate(_free_list_memory);

	CMemoryAllocator::Deallocate(_instances_memory);

	if (_motion_type == ERenderInstanceMotionType::Static)
		CGraphicsBuffers::GetStaticInstanceBuffer()->ReleaseStagingFrame(_instance_buffer_frame);
	else if (_motion_type == ERenderInstanceMotionType::Dynamic)
		CGraphicsBuffers::GetDynamicInstanceBuffer()->ReleaseStagingFrame(_instance_buffer_frame);
}

const triton::SRenderInstance* triton::XRenderGroup::AddInstance()
{
	if (!_free_list_capacity)
		return nullptr;

	const usize free_index = _free_list[--_free_list_capacity];

	if (_head == -1)
	{
		_head = free_index;
		_tail = free_index;

		_instances[free_index] = SRenderInstance(
			-1,
			-1,
			0
		);
	}
	else
	{
		_instances[_tail].next_node_index = free_index;

		_instances[free_index] = SRenderInstance(
			_tail,
			-1,
			_instances[_tail].instance_buffer_index + 1
		);

		_tail = free_index;
	}

	return &_instances[free_index];
}

void triton::XRenderGroup::RemoveInstance(const SRenderInstance* instance)
{
	if (!instance || _free_list_capacity >= _max_instance_count || instance < _instances)
		return;

	const s32 index_to_free = instance - _instances;

	const s32 tail_index = _tail;

	if (index_to_free != tail_index)
	{
		SRenderInstance& removed = _instances[index_to_free];
		SRenderInstance& tail = _instances[tail_index];

		const s32 left = removed.prev_node_index;
		const s32 right = removed.next_node_index;

		const s32 tail_prev = tail.prev_node_index;

		_instances[tail_prev].next_node_index = -1;

		tail.prev_node_index = left;
		tail.next_node_index = right == tail_index ? -1 : right;

		if (left != -1)
			_instances[left].next_node_index = tail_index;
		else
			_head = tail_index;

		if (right != -1 && right != tail_index)
			_instances[right].prev_node_index = tail_index;

		_tail = right == tail_index ? tail_index : tail_prev;

		tail.instance_buffer_index = removed.instance_buffer_index;
	}
	else
	{
		const s32 new_tail = _instances[tail_index].prev_node_index;

		_tail = new_tail;

		if (new_tail != -1)
			_instances[new_tail].next_node_index = -1;
		else
			_head = -1;
	}

	_free_list[_free_list_capacity++] = index_to_free;

	_instances[index_to_free].prev_node_index = -1;
	_instances[index_to_free].next_node_index = -1;
}