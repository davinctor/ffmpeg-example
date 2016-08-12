#include <jni.h>
#include <string.h>
#include "audio_player.h"

#define VOLUME_STEP 0.1f
#define MIN_VOLUME 0.0f
#define MAX_VOLUME 1.0f

float currentVolumeFactor = 0.5f;
AudioPlayer audioPlayer;

extern "C" {

JNIEXPORT void JNICALL
Java_tk_davinctor_jni3rdpartylibsample_MainActivity_createEngine(JNIEnv *env, jclass type) {
    createEngine(&audioPlayer);
}

JNIEXPORT void JNICALL
Java_tk_davinctor_jni3rdpartylibsample_MainActivity_destroyEngine(JNIEnv *env, jclass type) {
    destroyEngine(&audioPlayer);
}

JNIEXPORT void JNICALL
Java_tk_davinctor_jni3rdpartylibsample_MainActivity_createAssetAudioPlayer(JNIEnv *env,
                                                                           jobject instance,
                                                                           jobject assetManager,
                                                                           jstring fileName) {
    const char *utfFileName = env->GetStringUTFChars(fileName, NULL);

    AAssetManager *assetManagerNative = AAssetManager_fromJava(env, assetManager);
    initAssetAudioPlayer(&audioPlayer, assetManagerNative, utfFileName);

    env->ReleaseStringUTFChars(fileName, utfFileName);
}

JNIEXPORT void JNICALL
Java_tk_davinctor_jni3rdpartylibsample_MainActivity_play(JNIEnv *env, jclass type) {
    play(&audioPlayer);
}

JNIEXPORT void JNICALL
Java_tk_davinctor_jni3rdpartylibsample_MainActivity_pause(JNIEnv *env, jclass type) {
    pause(&audioPlayer);
}

JNIEXPORT void JNICALL
Java_tk_davinctor_jni3rdpartylibsample_MainActivity_volumePlus(JNIEnv *env, jclass type) {
    if (currentVolumeFactor < MAX_VOLUME)
    {
        currentVolumeFactor += VOLUME_STEP;
        setVolume(&audioPlayer, currentVolumeFactor);
    }
}

JNIEXPORT void JNICALL
Java_tk_davinctor_jni3rdpartylibsample_MainActivity_volumeMinus(JNIEnv *env, jclass type) {
    if (currentVolumeFactor > MIN_VOLUME)
    {
        currentVolumeFactor -= VOLUME_STEP;
        setVolume(&audioPlayer, currentVolumeFactor);
    }
}

} // extern "C" end;