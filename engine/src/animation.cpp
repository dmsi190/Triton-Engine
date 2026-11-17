// animation.cpp

#include "animation.hpp"
#include "skeleton_pool.hpp"

using namespace types;

triton::XAnimation::XAnimation(
    const SMemoryAllocation& mem_alloc,
    cContext* context,
    const std::string& name,
    f32 duration,
    f32 ticks_per_second,
    const std::vector<SAnimationKey>& keys
) : iObject(mem_alloc, context),
    _name(name),
    _duration(duration),
    _ticks_per_second(ticks_per_second),
    _keys(keys) {}

std::optional<triton::SAnimationFrame> triton::XAnimation::EvaluateFrame(
    const XSkeleton* skeleton,
    types::f32 time
)
{
    const auto& bones = skeleton->GetBones();

    SAnimationFrame frame = {};

    frame.bones.resize(bones.size());

    for (usize i = 0; i < bones.size(); ++i)
        frame.bones[i].transform = bones[i].localMatrix;

    if (_duration > 0.0f)
        time = std::fmod(time, _duration);

    for (const SAnimationKey& anim_key : _keys)
    {
        cVector3 pose_position = cVector3(0.0f);
        cQuaternion pose_rotation = cQuaternion();
        cVector3 pose_scale = cVector3(1.0f);

        // TODO: use proper math backend for lerp/slerp and other calculations
        // Rewrite this
        // |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
        // VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV

        if (!anim_key.position_keys.empty())
        {
            if (anim_key.position_keys.size() == 1)
            {
                pose_position = anim_key.position_keys[0].position;
            }
            else
            {
                usize next = 1;

                while (next < anim_key.position_keys.size() &&
                    time >= anim_key.position_keys[next].time)
                    ++next;

                if (next == anim_key.position_keys.size())
                {
                    pose_position = anim_key.position_keys.back().position;
                }
                else
                {
                    const auto& a = anim_key.position_keys[next - 1];
                    const auto& b = anim_key.position_keys[next];

                    f32 factor = (time - a.time) / (b.time - a.time);

                    pose_position = cVector3(glm::mix(a.position._vec, b.position._vec, factor));
                }
            }
        }

        if (!anim_key.rotation_keys.empty())
        {
            if (anim_key.rotation_keys.size() == 1)
            {
                pose_rotation = anim_key.rotation_keys[0].rotation;
            }
            else
            {
                usize next = 1;

                while (next < anim_key.rotation_keys.size() &&
                    time >= anim_key.rotation_keys[next].time)
                    ++next;

                if (next == anim_key.rotation_keys.size())
                {
                    pose_rotation = anim_key.rotation_keys.back().rotation;
                }
                else
                {
                    const auto& a = anim_key.rotation_keys[next - 1];
                    const auto& b = anim_key.rotation_keys[next];

                    f32 factor = (time - a.time) / (b.time - a.time);

                    pose_rotation = cQuaternion(glm::slerp(a.rotation._quat, b.rotation._quat, factor));
                }
            }
        }

        if (!anim_key.scale_keys.empty())
        {
            if (anim_key.scale_keys.size() == 1)
            {
                pose_scale = anim_key.scale_keys[0].scale;
            }
            else
            {
                usize next = 1;

                while (next < anim_key.scale_keys.size() &&
                    time >= anim_key.scale_keys[next].time)
                    ++next;

                if (next == anim_key.scale_keys.size())
                {
                    pose_scale = anim_key.scale_keys.back().scale;
                }
                else
                {
                    const auto& a = anim_key.scale_keys[next - 1];
                    const auto& b = anim_key.scale_keys[next];

                    f32 factor = (time - a.time) / (b.time - a.time);

                    pose_scale = cVector3(glm::mix(a.scale._vec, b.scale._vec, factor));
                }
            }
        }

        glm::mat4 matrix = glm::mat4(1.0f);
        matrix = glm::translate(matrix, pose_position._vec);
        matrix *= glm::mat4_cast(pose_rotation._quat);
        matrix = glm::scale(matrix, pose_scale._vec);

        SAnimationFrameBone fb = {};
        fb.transform = cMatrix4(matrix);

        frame.bones[anim_key.local_bone_index] = fb;
    }

    return frame;
}