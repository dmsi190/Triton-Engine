// skeleton.hpp

#pragma once

#include <vector>
#include "object.hpp"
#include "skeleton_bone.hpp"
#include "math.hpp"
#include "types.hpp"

namespace triton
{
	class cContext;

	class XSkeleton : public iObject
	{
		TRITON_OBJECT(XSkeleton)

		cMatrix4					_accumulatedRootTransform = cMatrix4();
		std::vector<SSkeletonBone>	_bones = {};

	public:
		explicit XSkeleton(
			const SMemAlloc& mem_alloc,
			cContext* context,
			const std::vector<SSkeletonBone>& bones,
			const cMatrix4& accumulatedRootTransform = cMatrix4()
		);

		~XSkeleton() override = default;

		inline const std::vector<SSkeletonBone>& GetBones() const { return _bones; }

		inline const cMatrix4& GetAccumulatedRootTransform() const { return _accumulatedRootTransform; }

		struct TGPULayout {};
	};
}