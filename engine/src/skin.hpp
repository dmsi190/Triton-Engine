// skin.hpp

#pragma once

#include <vector>
#include "handle.hpp"
#include "skeleton.hpp"
#include "math.hpp"
#include "animation_frame.hpp"
#include "skinned_bone.hpp"

namespace triton
{
	class XSkin : public iObject
	{
		TRITON_OBJECT(XSkin)

		SHandle									_skeleton;
		types::usize							_globSkinnedBoneBufferOffset = 0;
		std::vector<SHandle>					_skinnedBones;
	
	public:
		explicit XSkin(cContext* context, types::s32 poolIndex) : iObject(context, poolIndex) {}

		explicit XSkin(cContext* context, types::s32 poolIndex, const SHandle& skeleton);

		~XSkin() override;

		void Skin(const SAnimationFrame& frame);

		inline types::usize GetSkinnedBoneBufferOffset() const { return _globSkinnedBoneBufferOffset; }

		struct TGPULayout {};

	private:
		void CalculateBone(
			const std::vector<SSkeletonBone>& bones,
			types::usize boneIndex,
			const SAnimationFrame& frame,
			std::vector<cMatrix4>& totalTransform
		);
	};
}