#include <android/log.h>
#include <stdexcept>

#include <string.h>
#include <jni.h>
#include <inttypes.h>

extern "C" {
    #include "jni-3rd-party-lib.h"
}

#define LOG_TAG "jni_cpp_file"

#define LOGI(...) \
  ((void)__android_log_print(ANDROID_LOG_INFO, LOG_TAG,"%s"))

#define LOGE(...) \
  ((void)__android_log_print(ANDROID_LOG_ERROR, LOG_TAG,"%s"))

using namespace std;

void throwJavaException(JNIEnv *env, jobject instance, const char *errorMessage)
{
    jclass exceptionClass = env->FindClass("java/lang/RuntimeException");

    if (!exceptionClass)
    {
        exceptionClass = env->FindClass("java/lang/IllegalArgumentException");
    }

    env->ThrowNew(exceptionClass, errorMessage);
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env;

    if (vm->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR; // JNI version not supported.
    }

    return  JNI_VERSION_1_6;
}

extern "C" JNIEXPORT jstring JNICALL
Java_tk_davinctor_jni3rdpartylibsample_MainActivity_getNativeString(JNIEnv *env, jobject instance) {
    try {
        test();
    } catch (std::exception e) {
        LOGE(e.what());
        throwJavaException(env, instance, e.what());
    } catch (std::runtime_error error) {
        LOGE(error.what());
        throwJavaException(env, instance, error.what());
    }


    return env->NewStringUTF("Hello from fucking jni");
}