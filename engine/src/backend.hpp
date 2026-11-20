// backend.hpp

#pragma once

#include "object.hpp"

namespace triton
{
	class cContext;

	class IBackend : public iObject
	{
	public:
		explicit IBackend(const SMemAlloc& mem_alloc, cContext* context) : iObject(mem_alloc, context) {}

		virtual ~IBackend() = default;
	};
}