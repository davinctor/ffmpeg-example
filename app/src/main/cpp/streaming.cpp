#include <jni.h>
#include <android/log.h>
#include <android/bitmap.h>
#include <string>
#include <math.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
}

#define LOG_TAG "ffmpeg_wrapper"

#define LOGI(...) \
  ((void)__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))
#define LOGE(...) \
  ((void)__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))

typedef struct VideoState {
    AVFormatContext *pFormatContext;
    AVStream *pVideoStream;
    int videoStreamIdx;

    AVFrame *pFrame; // stored the decoded frame
    int frameInterval;
    int64_t  nextFrameTime; // track next frame display time
    int status;
} VideoState;

typedef struct VideoDisplayUtil {
    struct SwsContext *imgResampleCtx;
    AVFrame *pFrameRGBA;
    int width;
    int height;
    void* pBitmap; // use the bitmap as the pFrameRGBA buffer
    int frameNum;

} VideoDisplayUtil;

const char* gVideoFilename;
VideoState  *gVideoState;
VideoDisplayUtil *gVideoDisplayUtil;

namespace code {
    int SUCCESS = 0;
    int ERROR_ALLOCATE_MEMORY = -1;
    int ERROR_OPEN_FILE = -2;
    int ERROR_FIND_STREAM_INFO = -3;
    int ERROR_VIDEO_STREAM_NOT_FOUND = -4;
    int ERROR_CODEC_NOT_FOUND = -5;
    int ERROR_OPEN_CODEC = -6;
    int ERROR_ANDROID_BITMAP_GET_INFO = -7;
    int ERROR_ANDROID_BITMAP_WRONG_FORMAT = -8;
    int ERROR_ANDROID_BITMAP_LOCK_PIXELS = -9;
    int ERROR_INIT_VIDEO_FRAME_CONVERSION_CONTEXT = -10;
}

/**
 * Throw java exception with specified error message
 */
void throwJavaException(JNIEnv *env, jobject instance, const char *errorMessage)
{
    jclass exceptionClass = env->FindClass("java/lang/RuntimeException");

    if (!exceptionClass)
    {
        exceptionClass = env->FindClass("java/lang/IllegalArgumentException");
    }

    env->ThrowNew(exceptionClass, errorMessage);
}

/**
 * Init and load all necessary codecs, open file, find video stream and open codec.
 * Saved all info to global variables.
 */
extern "C" JNIEXPORT jint JNICALL
Java_tk_davinctor_jni3rdpartylibsample_FFmpegWrapper_init(JNIEnv *env,
                                                          jobject instance,
                                                          jstring filename)
{
    gVideoFilename = env->GetStringUTFChars(filename, NULL);

    // Register all formats and codecs
    av_register_all();

    // Allocate memory for gVideoState
    gVideoState = (VideoState *) av_mallocz(sizeof(VideoState));

    if (gVideoState == NULL) {
        LOGE("Can't allocate memory VideoState size");
        return code::ERROR_ALLOCATE_MEMORY;
    }

    int resultCode = avformat_open_input(&gVideoState->pFormatContext, gVideoFilename, NULL, NULL);
    // Open the video file
    if (resultCode < 0)
    {
        LOGE("Couldn't open file %s, because %s", gVideoFilename, av_err2str(resultCode));
        return code::ERROR_OPEN_FILE;
    }

    resultCode = avformat_find_stream_info(gVideoState->pFormatContext, NULL);
    // Retrieve stream info
    if (resultCode < 0) {
        LOGE("Couldn't find stream information, because %s", av_err2str(resultCode));
        return code::ERROR_FIND_STREAM_INFO;
    }

    // Dump information about file onto standard error
    av_dump_format(gVideoState->pFormatContext, 0, gVideoFilename, 0 /* is output = false*/);

    // Find video stream
    gVideoState->videoStreamIdx = -1;
    for (int i = 0; i < gVideoState->pFormatContext->nb_streams; i++)
    {
        if (gVideoState->pFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            LOGI("Found video stream");
            gVideoState->videoStreamIdx = i;
            gVideoState->pVideoStream = gVideoState->pFormatContext->streams[i];
            break;
        }
    }

    if (gVideoState->videoStreamIdx == -1)
    {
        LOGE("Didn't find a video stream");
        return code::ERROR_VIDEO_STREAM_NOT_FOUND;
    }

    AVCodecContext *pCodecContext = gVideoState->pVideoStream->codec;
    // Find codec decoder
    AVCodec *pCodec = avcodec_find_decoder(gVideoState->pVideoStream->codecpar->codec_id);

    if (pCodec == NULL)
    {
        LOGE("Codec not found");
        return code::ERROR_CODEC_NOT_FOUND;
    }

    // Open codec
    resultCode = avcodec_open2(pCodecContext, pCodec, NULL);

    if (resultCode < 0) {
        LOGE("Couldn't open codec");
        return code::ERROR_OPEN_CODEC;
    }

    return code::SUCCESS;
}

/**
 * Free resources and close video file
 */
extern "C" JNIEXPORT jint JNICALL
Java_tk_davinctor_jni3rdpartylibsample_FFmpegWrapper_finish(JNIEnv *env,
                                                            jobject instance)
{
    // Close codec
    avcodec_close(gVideoState->pVideoStream->codec);
    // Close video file
    avformat_close_input(&gVideoState->pFormatContext);
    // Free resources
    av_free(gVideoState);

    return code::SUCCESS;
}

/**
 * Returns resolution of video file
 */
extern "C" JNIEXPORT jintArray JNICALL
Java_tk_davinctor_jni3rdpartylibsample_FFmpegWrapper_getVideoResolution(JNIEnv *env,
                                                                        jobject instance)
{
    AVCodecParameters *pCodecParameters = gVideoState->pVideoStream->codecpar;
    jint res[2];
    res[0] = pCodecParameters->width;
    res[1] = pCodecParameters->height;

    jintArray resolution = env->NewIntArray(2);
    env->SetIntArrayRegion(resolution, 0, 2, res);
    return resolution;
}

/**
 * Get duration of video file in seconds
 */
extern "C" JNIEXPORT jdouble JNICALL
Java_tk_davinctor_jni3rdpartylibsample_FFmpegWrapper_getVideoDuration(JNIEnv *env,
                                                                      jobject instance)
{
    jdouble duration = (jdouble) (gVideoState->pFormatContext->duration / AV_TIME_BASE);
    return duration;
}

/**
 * Returns frame rate of video file
 */
extern "C" JNIEXPORT jdouble JNICALL
Java_tk_davinctor_jni3rdpartylibsample_FFmpegWrapper_getVideoFrameRate(JNIEnv *env,
                                                                       jobject instance)
{
    jdouble frameRate = -1;
    AVStream *pVideoStream = gVideoState->pVideoStream;
    if (pVideoStream->avg_frame_rate.den > 0 && pVideoStream->avg_frame_rate.num > 0)
    {
        frameRate = av_q2d(pVideoStream->avg_frame_rate);
    } else if (pVideoStream->r_frame_rate.den > 0 && pVideoStream->r_frame_rate.num > 0)
    {
        frameRate = av_q2d(pVideoStream->r_frame_rate);
    } else if (pVideoStream->time_base.den > 0 && pVideoStream->time_base.num > 0)
    {
        frameRate = av_q2d(pVideoStream->time_base);
    } else if (pVideoStream->codec->time_base.den > 0 && pVideoStream->codec->time_base.num > 0)
    {
        frameRate = av_q2d(pVideoStream->codec->time_base);
    }

    return frameRate;
}

int getFrameInterval() {
    double frameInterval = 0;
    AVStream *pVideoStream = gVideoState->pVideoStream;
    if (pVideoStream->avg_frame_rate.den > 0 && pVideoStream->avg_frame_rate.num > 0)
    {
        frameInterval = 1000 / av_q2d(pVideoStream->avg_frame_rate);
    } else if (pVideoStream->r_frame_rate.den > 0 && pVideoStream->r_frame_rate.num > 0)
    {
        frameInterval = 1000 / av_q2d(pVideoStream->r_frame_rate);
    } else if (pVideoStream->time_base.den > 0 && pVideoStream->time_base.num > 0)
    {
        frameInterval = 1000 * av_q2d(pVideoStream->time_base);
    } else if (pVideoStream->codec->time_base.den > 0 && pVideoStream->codec->time_base.num > 0)
    {
        frameInterval = 1000 * av_q2d(pVideoStream->codec->time_base);
    }

    if (frameInterval < 20)
    {
        frameInterval = 20; // Min interval => max frame rate 1000 / 20 = 50 fps;
    }
    else if (frameInterval > 100)
    {
        frameInterval = 100; // Max interval => min frame rate 1000 / 100 = 10 fps;
    }

    return (int) round(frameInterval);
}

extern "C" JNIEXPORT jint JNICALL
Java_tk_davinctor_jni3rdpartylibsample_FFmpegWrapper_prepareDisplay(JNIEnv *env, jobject pObj,
                                                                    jobject pBitmap,
                                                                    jint width, jint height)
{
    gVideoDisplayUtil = (VideoDisplayUtil *) av_mallocz(sizeof(VideoDisplayUtil));
    if (gVideoDisplayUtil == NULL)
    {
        LOGE("Couldn't allocate memory VideoDisplayUtil size");
        return code::ERROR_ALLOCATE_MEMORY;
    }

    gVideoState->pFrame = av_frame_alloc();
    if (gVideoState->pFrame == NULL)
    {
        LOGE("Couldn't allocate memory for AVFrame size for frame");
        return code::ERROR_ALLOCATE_MEMORY;
    }

    gVideoState->frameInterval = getFrameInterval();
    gVideoDisplayUtil->frameNum = 0;
    gVideoDisplayUtil->width = width;
    gVideoDisplayUtil->height = height;

    gVideoDisplayUtil->pFrameRGBA = av_frame_alloc();
    if (gVideoDisplayUtil->pFrameRGBA == NULL)
    {
        LOGE("Couldn't allocate memory AVFrame size for frameRGBA");
        return code::ERROR_ALLOCATE_MEMORY;
    }

    AndroidBitmapInfo bitmapInfo;

    // Retrieve information about the bitmap
    int resultCode = AndroidBitmap_getInfo(env, pBitmap, &bitmapInfo);
    if (resultCode < 0)
    {
        LOGE("AndroidBitmap_getInfo() failed.");
        return code::ERROR_ANDROID_BITMAP_GET_INFO;
    }

    // Lock the pixel buffer and retrieve a pointer to it
    resultCode = AndroidBitmap_lockPixels(env, pBitmap, &gVideoDisplayUtil->pBitmap);
    if (resultCode < 0)
    {
        LOGE("AndroidBitmap_lockPixels() failed.");
        return code::ERROR_ANDROID_BITMAP_LOCK_PIXELS;
    }

    // For android use the bitmap buffer as the buffer for frameRGBA
    av_image_fill_arrays
            (gVideoDisplayUtil->pFrameRGBA->data,
             gVideoDisplayUtil->pFrameRGBA->linesize,
             (const uint8_t *) gVideoDisplayUtil->pBitmap,
             AV_PIX_FMT_RGBA,
             width,
             height,
             1);
    gVideoDisplayUtil->imgResampleCtx = sws_getContext
            (gVideoState->pVideoStream->codecpar->width,
             gVideoState->pVideoStream->codecpar->height,
             gVideoState->pVideoStream->codec->pix_fmt,
             width,
             height,
             AV_PIX_FMT_RGBA,
             SWS_BICUBIC,
             NULL,
             NULL,
             NULL);
    if (gVideoDisplayUtil->imgResampleCtx == NULL)
    {
        LOGE("Error initialize the video frame conversion context");
        return code::ERROR_INIT_VIDEO_FRAME_CONVERSION_CONTEXT;
    }

    gVideoState->nextFrameTime = av_gettime();

    return code::SUCCESS;
}

extern "C" JNIEXPORT int JNICALL
Java_tk_davinctor_jni3rdpartylibsample_FFmpegWrapper_getVideoFrame(JNIEnv *env,
                                                                   jobject instance)
{
    AVPacket packet;
    // Read frames and decode them
    int isFrameFinished;
    int resultCode;
    while ( !(gVideoState->status) && av_read_frame(gVideoState->pFormatContext, &packet) >= 0)
    {
        if (gVideoState->videoStreamIdx == packet.stream_index)
        {
            resultCode = avcodec_decode_video2(gVideoState->pVideoStream->codec,
                                               gVideoState->pFrame, &isFrameFinished, &packet);
            if (resultCode < 0)
            {
                LOGE("Couldn't decode video, because %s", av_err2str(resultCode));
            }
            else if (isFrameFinished)
            {
                sws_scale(gVideoDisplayUtil->imgResampleCtx,
                          (const uint8_t *const *) gVideoState->pFrame->data,
                          gVideoState->pFrame->linesize,
                          0,
                          gVideoState->pVideoStream->codecpar->height,
                          gVideoDisplayUtil->pFrameRGBA->data,
                          gVideoDisplayUtil->pFrameRGBA->linesize);
                int64_t curTime = av_gettime();
                if (gVideoState->nextFrameTime - curTime > 20 * 1000)
                {
                    av_usleep((unsigned int) (gVideoState->nextFrameTime - curTime));
                }
                ++gVideoDisplayUtil->frameNum;
                gVideoState->nextFrameTime += gVideoState->frameInterval * 1000;
                return gVideoDisplayUtil->frameNum;
            }
        }
        av_packet_unref(&packet);
    }

    return code::SUCCESS;
}