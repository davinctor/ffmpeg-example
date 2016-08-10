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

#define bool int8_t
#define TRUE (int8_t) 1
#define FALSE (int8_t) 0

#define LOG_TAG "AudioPlayerJNI"

#define LOGI(...) \
  ((void)__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))
#define LOGE(...) \
  ((void)__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))

enum codes {
    SUCCESS = 0,
    ERROR_CREATE_ENGINE_OBJECT = -1,
    ERROR_REALIZE_ENGINE_OBJECT = -2,
    ERROR_GET_INTERFACE_ENGINE = -3,
    ERROR_CREATE_OUTPUT_MIX = -4,
    ERROR_REALIZE_OUTPUT_MIX = -5,
    ERROR_CREATE_AUDIO_PLAYER = -6,
    ERROR_REALIZE_AUDIO_PLAYER = -7,
    ERROR_GET_INTERFACE_PLAY = -8,
    ERROR_GET_INTERFACE_BUFFER_QUEUE = -9,
    ERROR_REGISTER_BUFFER_QUEUE_CALLBACK = -10,
    ERROR_SET_PLAYING_STATE_TO_PLAYING = -11,
    ERROR_SET_PLAYING_STATE_TO_PAUSED = -12,
    ERROR_SET_PLAYING_STATE_TO_STOPPED = -13,
    ERROR_SET_LOOP = -14,
};

typedef struct AudioPlayer {

    // engine interfaces
    SLObjectItf engineObject;
    SLEngineItf engine;

    // Output mix interfaces
    SLObjectItf  outputMixObject;
    SLEnvironmentalReverbItf  outputEnvironmentalReverbItf;

    // Player objects
    SLObjectItf playerObject;
    SLPlayItf playerPlay;
    SLSeekItf  playerSeek;
    SLVolumeItf playerVolume;
    SLMuteSoloItf muteSolo;

    // Buffer queue interfaces
    SLAndroidSimpleBufferQueueItf bufferQueue;

    // Effects
    SLEffectSendItf effectSend;

    // Others
    int bufferSize;
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

/**
 * Init all resources for OpenSL
 */
int createEngine(AudioPlayer **audioPlayer);

/**
 * Free all resources by OpenSL
 */
int destroyEngine(AudioPlayer **audioPlayer);

/**
 * Create buffer queue
 */
int initAudioPlayer(AudioPlayer **audioPlayer, int sampleRate, int bufferSize);

int play(AudioPlayer **audioPlayer);

int pause(AudioPlayer **audioPlayer);

int stop(AudioPlayer **audioPlayer);

/**
 * This callback handler is called every time a buffer finishes playing
 */
void onBufferPlayFinished(SLAndroidSimpleBufferQueueItf bufferQueue, void *context);

int setLooping(AudioPlayer **audioPlayer, bool isLooping);

SLVolumeItf getVolume();
void setVolume(int milliBel);

void setMute(bool isMute);

void setEnableStereoPosition(bool isEnable);

void setStereoPosition(int perMille);

#endif //FFMPEG_EXAMPLE_AUDIO_PLAYER_H
