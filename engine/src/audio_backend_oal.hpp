// audio_backend_oal.hpp
/*
#pragma once

#include <AL/al.h>
#include <AL/alc.h>
#include "audio_backend.hpp"

namespace triton
{
    class cContext;

    class cAudioBackendOAL final : public iAudioBackend
    {
        ALCdevice* _alDevice = nullptr;
        ALCcontext* _alContext = nullptr;

    public:
        explicit cAudioBackendOAL(cContext* context);
        virtual ~cAudioBackendOAL() override final;

        virtual SAudioResource CreateSound(
            cAudio::eDataFormat dataFormat,
            const types::u8* data,
            types::usize dataByteSize,
            types::usize sampleRate
        ) override final;
        virtual void DestroySound(SAudioResource& sound) override final;
        virtual void PlaySound(SAudioResource& sound) override final;
        virtual void StopSound(SAudioResource& sound) override final;
        virtual void SetSoundPosition(SAudioResource& sound, const cVector3& position) override final;
        virtual void SetSoundVelocity(SAudioResource& sound, const cVector3& velocity) override final;
        virtual void SetListenerPosition(const cVector3& position) override final;
        virtual void SetListenerVelocity(const cVector3& velocity) override final;
        virtual void SetListenerOrientation(const cVector3& at, const cVector3& up) override final;
    };
}*/