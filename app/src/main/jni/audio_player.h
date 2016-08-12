//
// Created by victor_ponomarenko on 09.08.16.
//

#ifndef FFMPEG_EXAMPLE_AUDIO_PLAYER_H
#define FFMPEG_EXAMPLE_AUDIO_PLAYER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "math.h"
#include <android/log.h>
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

typedef enum {
    NO_VALUE = -1
} value;

typedef enum {
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
    ERROR_SET_VOLUME = -15,
    ERROR_ENQUEUE_BUFFER_DATA = -16,
    ERROR_OPEN_ASSET = -17,
    ERROR_OPEN_FILE_DESCRIPTOR = -18,
    ERROR_GET_INTERFACE_SEEK = -19,
    ERROR_GET_INTERFACE_MUTE_SOLO = -20,
    ERROR_GET_INTERFACE_VOLUME = -21
} codes;

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
    //pthread_mutex_t audioEngineLock;

    // aux effect on the output mix, used by the buffer queue player
    SLEnvironmentalReverbSettings reverbSettings;

    short *nextBuffer;
    unsigned nextSize;
    int nextCount;

    /**
     * This callback handler is called every time a buffer finishes playing
     */
    void (*onBufferPlayFinished) (SLAndroidSimpleBufferQueueItf bufferQueue, void *context);

    void (*audioCallback) (void *userdata, uint8_t *stream, int length);
} AudioPlayer;

/**
 * Init all resources for OpenSL
 */
int createEngine(AudioPlayer *audioPlayer);

/**
 * Free all resources by OpenSL
 */
int destroyEngine(AudioPlayer *audioPlayer);

/**
 * Create audio player which playing file from asset
 */
int initAssetAudioPlayer(AudioPlayer *audioPlayer, AAssetManager *assetManager, const char* fileName);

/**
 * Create audio player which playing buffer data
 */
int initBufferAudioPlayer(AudioPlayer *audioPlayer, int sampleRate, int bufferSize);

int play(AudioPlayer *audioPlayer);

int pause(AudioPlayer *audioPlayer);

int stop(AudioPlayer *audioPlayer);

int enqueueBufferData(AudioPlayer *audioPlayer, int16_t *data, int size);

int setLooping(AudioPlayer *audioPlayer, bool isLooping);

int getVolume(AudioPlayer *audioPlayer);
int setVolume(AudioPlayer *audioPlayer, float volumeFactor);

void setMute(bool isMute);

void setEnableStereoPosition(bool isEnable);

void setStereoPosition(int perMille);

#ifdef __cplusplus
}
#endif

#endif //FFMPEG_EXAMPLE_AUDIO_PLAYER_H
