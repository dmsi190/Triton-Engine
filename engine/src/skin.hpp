// skin.hpp

#pragma once

#include <vector>
#include "skeleton.hpp"
#include "math.hpp"
#include "animation_frame.hpp"

namespace triton
{
	class XSkin : public iObject
	{
		TRITON_OBJECT(XSkin)

		XSkeleton*								_skeleton;
		types::usize							_globSkinnedBoneBufferOffset = 0;
		std::vector<void*>						_skinnedBones;
	
	public:
		explicit XSkin(const SMemAlloc& mem_alloc, cContext* context, const XSkeleton* skeleton);

		~XSkin() override;

		void Skin(const SAnimationFrame& frame);

		inline types::usize GetSkinnedBoneBufferOffset() const { return _globSkinnedBoneBufferOffset; }

	private:
		void CalculateBone(
			const std::vector<SSkeletonBone>& bones,
			types::usize boneIndex,
			const SAnimationFrame& frame,
			std::vector<cMatrix4>& totalTransform
		);
	};
}