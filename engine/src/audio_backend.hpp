// audio_backend.hpp
/*
#pragma once

#include "audio.hpp"
#include "audio_resource.hpp"
#include "backend.hpp"
#include "math.hpp"

namespace triton
{
    class cContext;

    class iAudioBackend : public iBackend
    {
        TRITON_CLASS_NAME(iAudioBackend)

    public:
        explicit iAudioBackend(cContext* context);
        virtual ~iAudioBackend() override = default;

        virtual SAudioResource CreateSound(
            cAudio::eDataFormat dataFormat,
            const types::u8* data,
            types::usize dataByteSize,
            types::usize sampleRate
        ) = 0;
        virtual void DestroySound(SAudioResource& sound) = 0;
        virtual void PlaySound(SAudioResource& sound) = 0;
        virtual void StopSound(SAudioResource& sound) = 0;
        virtual void SetSoundPosition(SAudioResource& sound, const cVector3& position) = 0;
        virtual void SetSoundVelocity(SAudioResource& sound, const cVector3& velocity) = 0;
        virtual void SetListenerPosition(const cVector3& position) = 0;
        virtual void SetListenerVelocity(const cVector3& velocity) = 0;
        virtual void SetListenerOrientation(const cVector3& at, const cVector3& up) = 0;
    };
}
*/