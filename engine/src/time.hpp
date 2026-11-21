// time.hpp

#pragma once

#include <chrono>
#include <string>
#include <typeinfo>
#include "log.hpp"
#include "subsystem.hpp"
#include "types.hpp"

namespace triton
{
	class cContext;

	class cTime : public XSubsystem
	{
		TRITON_OBJECT(cTime)

		std::chrono::steady_clock::time_point _timepointLast;
		types::f32 _deltaTime = 0.0f;

	public:
		explicit cTime(const SMemAlloc& mem_alloc, cContext* context);

		~cTime() override = default;

		void BeginFrame();

		void Update();

		void EndFrame();

		inline types::f32 GetDeltaTime() const { return _deltaTime; }
	};
}