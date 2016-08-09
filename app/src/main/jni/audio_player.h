//
// Created by victor_ponomarenko on 09.08.16.
//

#ifndef FFMPEG_EXAMPLE_AUDIO_PLAYER_H
#define FFMPEG_EXAMPLE_AUDIO_PLAYER_H

#include <android/log.h>
#include <jni.h>
#include <string.h>
#include <pthread.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#define LOG_TAG "AudioPlayerJNI"

#define LOGI(...) \
  ((void)__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))
#define LOGE(...) \
  ((void)__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))

enum codes {
    ERROR_CREATE_ENGINE_OBJECT = -1
};

typedef struct AudioPlayer {

    // engine interfaces
    SLObjectItf *engineObject;
    SLEngineItf *engine;

    // Output mix interfaces
    SLObjectItf  *outputMixObject;
    SLEnvironmentalReverbItf  *outputEnvironmentalReverbItf;

    // Player objects
    SLObjectItf *playerObject;
    SLPlayItf *playerPlay;
    SLSeekItf  *playerSeek;
    SLVolumeItf *playerVolume;
    SLMuteSoloItf *muteSolo;

    // Buffer queue interfaces
    SLAndroidSimpleBufferQueueItf *bufferQueue;

    // Effects
    SLEffectSendItf *effectSend;

    // Others
    SLmilliHertz sampleRate;
    short *resampledBuffer;

    // mutex to guard against re-entrance to record and playback
    pthread_mutex_t audioEngineLock;

    // aux effect on the output mix, used by the buffer queue player
    SLEnvironmentalReverbSettings reverbSettings;

    short *nextBuffer;
    unsigned nextSize;
    int nextCount;
} AudioPlayer;

int createEngine();
int destroyEngine();

short* createResampledBuffer(uint32_t idx, uint32_t srcRate, unsigned *size);
void releaseResampledBuffer();

void createBufferQueue(jint sampleRate, jint bufferSize);

/**
 * This callback handler is called every time a buffer finishes playing
 */
void onBufferPlayFinished(SLAndroidSimpleBufferQueueItf bufferQueue, void *context);

void setLooping(int8_t isLooping);

SLVolumeItf getVolume();
void setVolume(int milliBel);

void setMute(int8_t isMute);

void setEnableStereoPosition(int8_t isEnable);

void setStereoPosition(int perMille);

void play();
void pause();
void stop();

#endif //FFMPEG_EXAMPLE_AUDIO_PLAYER_H
