// world.hpp

#pragma once

#include "subsystem.hpp"
#include "world_object.hpp"

namespace triton
{
	class UWorld : public XSubsystem
	{
		TRITON_OBJECT(UWorld)

	public:
		explicit UWorld(const SMemAlloc& mem_alloc, cContext* context) : XSubsystem(mem_alloc, context) {}
		
		~UWorld() override = default;
	};
}