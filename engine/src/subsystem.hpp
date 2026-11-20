// subsystem.hpp

#pragma once

#include "object.hpp"

namespace triton
{
	class cContext;

	class XSubsystem : public iObject
	{
		TRITON_OBJECT(XSubsystem)

	public:
		explicit XSubsystem(const SMemAlloc& mem_alloc, cContext* context) : iObject(mem_alloc, context) {}

		~XSubsystem() = default;
	};
}