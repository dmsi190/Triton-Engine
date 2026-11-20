// skeleton.cpp

#include "skeleton.hpp"

triton::XSkeleton::XSkeleton(
	const SMemAlloc& mem_alloc,
	cContext* context,
	const std::vector<SSkeletonBone>& bones,
	const cMatrix4& accumulatedRootTransform
) : iObject(mem_alloc, context),
	_bones(bones),
	_accumulatedRootTransform(accumulatedRootTransform) {}