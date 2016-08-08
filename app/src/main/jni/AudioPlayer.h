//
// Created by victor_ponomarenko on 08.08.16.
//

#ifndef FFMPEG_EXAMPLE_AUDIO_PLAYER_H
#define FFMPEG_EXAMPLE_AUDIO_PLAYER_H

#include <jni.h>
#include <string.h>
#include <pthread.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

class AudioPlayer {
private:
    // engine interfaces
    SLObjectItf engineObject;
    SLEngineItf engine;

    // Output mix interfaces
    SLObjectItf  outputMixObject;
    SLEnvironmentalReverbItf  outputEnvironmentalReverbItf;

    // Player objects
    SLObjectItf playerObject;
    SLPlayItf playerPlay;
    SLVolumeItf playerVolume;

    // Buffer queue interfaces
    SLAndroidSimpleBufferQueueItf bufferQueue;

    // Effects
    SLEffectSendItf effectSend;
    SLMuteSoloItf muteSolo;

    // Others
    SLmilliHertz sampleRate;
    short *resampledBuffer;

    // mutex to guard against re-entrance to record and playback
    pthread_mutex_t audioEngineLock = PTHREAD_COND_INITIALIZER;

    // aux effect on the output mix, used by the buffer queue player
    SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_BATHROOM;

    short *nextBuffer;
    unsigned nextSize;
    int nextCount;

    short* createResampledBuffer(uint32_t idx, uint32_t srcRate, unsigned *size);
    void releaseResampledBuffer();

    void createBufferQueue(jint sampleRate, jint bufferSize);

    /**
     * This callback handler is called every time a buffer finishes playing
     */
    void onBufferPlayFinished(SLAndroidSimpleBufferQueueItf bufferQueue, void *context);

    void setLooping(bool isLooping);

    SLVolumeItf  getVolume();
    void setVolume(int milliBel);

    void setMute(bool isMute);

    void setEnableStereoPosition(bool isEnable);

    void setStereoPosition(int perMille);

    AudioPlayer();
    ~AudioPlayer();

};


#endif //FFMPEG_EXAMPLE_AUDIO_PLAYER_H
