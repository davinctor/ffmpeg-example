
#include "audio_player.h"

int createEngine(AudioPlayer** audioPlayer)
{
    SLObjectItf engineObject = (*audioPlayer)->engineObject;
    SLEngineItf engine = (*audioPlayer)->engine;

    SLresult  result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    if (result != SL_RESULT_SUCCESS)
    {
        LOGE("Couldn't create engine object");
        return codes::ERROR_CREATE_ENGINE_OBJECT;
    }
    else
    {
        LOGI("Engine object created");
    }

    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE); // not async
    if (result != SL_RESULT_SUCCESS)
    {
        LOGE("Couldn't realize engine object");
        return codes::ERROR_REALIZE_ENGINE_OBJECT;
    }
    else
    {
        LOGI("Engine object realized");
    }

    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engine);
    if (result != SL_RESULT_SUCCESS)
    {
        LOGE("Couldn't GetInterface of engine");
        return codes::ERROR_GET_INTERFACE_ENGINE;
    }
    else
    {
        LOGI("Interface Engine got");
    }

    SLObjectItf outputMixObject = (*audioPlayer)->outputMixObject;
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb = (*audioPlayer)->outputEnvironmentalReverbItf;

    // create output mix, with environmental reverb specified as non-required interface
    const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req[1] = {SL_BOOLEAN_FALSE};
    result = (*engine)->CreateOutputMix(engine, &outputMixObject, 1, ids, req);
    if (result != SL_RESULT_SUCCESS)
    {
        LOGE("Couldn't create output mix object");
        return codes::ERROR_CREATE_OUTPUT_MIX;
    }
    else
    {
        LOGI("OutputMix object created");
    }

    // realize output mix object
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS)
    {
        LOGE("Couldn't realize output mix object");
        return codes::ERROR_REALIZE_OUTPUT_MIX;
    }
    else
    {
        LOGI("OutputMix object realized");
    }

    // get the environmental reverb interface
    // this could fail if the environmental reverb effect is not available,
    // either because the feature is not present, excessive CPU load, or
    // the required MODIFY_AUDIO_SETTINGS permission was not requested and granted
    result = (*outputMixObject)->GetInterface(outputMixObject,
                                               SL_IID_ENVIRONMENTALREVERB,
                                               &outputMixEnvironmentalReverb);

    if (result != SL_RESULT_SUCCESS)
    {
        // just log unsuccessful result codes for environmental reverb, as it is optional
        LOGE("Couldn't get interface EnvironmentalReverb");
    }
    else
    {
        LOGI("Interface EnvironmentalReverb got");
    }

    return codes::SUCCESS;
}

int destroyEngine(AudioPlayer **audioPlayer)
{
    SLObjectItf playerObject = (*audioPlayer)->playerObject;
    if (playerObject != NULL) {
        (*playerObject)->Destroy(playerObject);
        (*audioPlayer)->playerPlay = NULL;
        (*audioPlayer)->playerSeek = NULL;
        (*audioPlayer)->playerVolume = NULL;
        (*audioPlayer)->bufferQueue = NULL;
        (*audioPlayer)->effectSend = NULL;
        (*audioPlayer)->muteSolo = NULL;
        LOGI("Player destroyed");
    }

    SLObjectItf outputMixObject = (*audioPlayer)->outputMixObject;
    if (outputMixObject != NULL)
    {
        (*outputMixObject)->Destroy(outputMixObject);
        (*audioPlayer)->outputMixObject = NULL;
        (*audioPlayer)->outputEnvironmentalReverbItf = NULL;
        LOGI("OutputMix destroyed");
    }

    SLObjectItf engineObject = (*audioPlayer)->engineObject;
    if (engineObject != NULL)
    {
        (*engineObject)->Destroy(engineObject);
        (*audioPlayer)->engineObject = NULL;
        (*audioPlayer)->engine = NULL;
        LOGI("Engine destroyed");
    }

    return codes::SUCCESS;
}

int initAudioPlayer(AudioPlayer **audioPlayer, int sampleRate, int bufferSize)
{
    if (sampleRate >= 0 && bufferSize >= 0)
    {
        (*audioPlayer)->sampleRate = (SLmilliHertz) (sampleRate * 1000);
        /*
         * device native buffer size is another factor to minimize audio latency, not used in this
         * sample: we only play one giant buffer here
         */
        (*audioPlayer)->bufferSize = bufferSize;
    }

    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue bufferQueueLocator = {SL_DATALOCATOR_ANDROIDBUFFERQUEUE, 2};

    SLDataFormat_PCM formatPcm = {
            SL_DATAFORMAT_PCM,          //    SLuint32      formatType;
            1,                          //    SLuint32 		numChannels;
            SL_SAMPLINGRATE_8,          //    SLuint32 		samplesPerSec;
            SL_PCMSAMPLEFORMAT_FIXED_16,//    SLuint32 		bitsPerSample;
            SL_PCMSAMPLEFORMAT_FIXED_16,//    SLuint32 		containerSize;
            SL_SPEAKER_FRONT_CENTER,    //    SLuint32 		channelMask;
            SL_BYTEORDER_LITTLEENDIAN   //    SLuint32		endianness;
    };

    /*
     * Enable Fast Audio when possible: once we set the same rate to be the native, fast audio path
     *  will be triggered
     */
    if ((*audioPlayer)->sampleRate > 0)
    {
        formatPcm.samplesPerSec = (*audioPlayer)->sampleRate; //sample rate in mili second
    }

    SLDataSource audioSource = {
            &bufferQueueLocator,// void *pLocator;
            &formatPcm          // void *pFormat;
    };

    // Configure audio sink
    SLDataLocator_OutputMix outputMix = {
            SL_DATALOCATOR_OUTPUTMIX,       // SLuint32     locatorType;
            (*audioPlayer)->outputMixObject // SLObjectItf	outputMix;
    };
    SLDataSink audioSink = {
            &outputMix, // void *pLocator;
            NULL        // void *pFormat;
    };

    /*
     * create audio player:
     *     fast audio does not support when SL_IID_EFFECTSEND is required, skip it
     *     for fast audio case
     */
    SLuint32 numberOfInterfaces = 4;
    const SLInterfaceID ids[numberOfInterfaces] = {
            SL_IID_BUFFERQUEUE,
            SL_IID_VOLUME,
            SL_IID_EFFECTSEND,
            SL_IID_MUTESOLO
    };
    const SLboolean requiredInterfaces[numberOfInterfaces] = {
            SL_BOOLEAN_TRUE,
            SL_BOOLEAN_TRUE,
            SL_BOOLEAN_TRUE,
            SL_BOOLEAN_FALSE
    };

    // get audio player object
    SLEngineItf engine = (*audioPlayer)->engine;
    SLObjectItf playerObject = (*audioPlayer)->playerObject;
    SLresult result = (*engine)->
            CreateAudioPlayer(engine, &playerObject, &audioSource, &audioSink, numberOfInterfaces,
                              ids, requiredInterfaces);
    if (result != SL_RESULT_SUCCESS)
    {
        LOGE("Couldn't create audio player");
        return codes::ERROR_CREATE_AUDIO_PLAYER;
    }

    // realize player
    result = (*playerObject)->Realize(playerObject, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS)
    {
        LOGE("Couldn't realize audio player");
        return codes::ERROR_REALIZE_AUDIO_PLAYER;
    }

    // get play interface
    result = (*playerObject)->GetInterface(playerObject, SL_IID_PLAY, &(*audioPlayer)->playerPlay);
    if (result != SL_RESULT_SUCCESS)
    {
        LOGE("Couldn't get interface of Play");
        return codes::ERROR_GET_INTERFACE_PLAY;
    }

    // get buffer queue interface
    SLAndroidSimpleBufferQueueItf bufferQueue = (*audioPlayer)->bufferQueue;
    result = (*playerObject)->GetInterface(playerObject, SL_IID_BUFFERQUEUE, &bufferQueue);
    if (result != SL_RESULT_SUCCESS)
    {
        LOGE("Couldn't get interface BufferQueue");
        return codes::ERROR_GET_INTERFACE_BUFFER_QUEUE;
    }
    // register callback to a buffer queue
    result = (*bufferQueue)->RegisterCallback(bufferQueue, onBufferPlayFinished, NULL);
    if (result != NULL)
    {
        LOGE("Couldn't register callback for BufferQueue");
        return codes::ERROR_REGISTER_BUFFER_QUEUE_CALLBACK;
    }

    // get the effect send interface
    if ((*audioPlayer)->sampleRate == 0)
    {
        result = (*playerObject)->GetInterface(playerObject, SL_IID_EFFECTSEND, &(*audioPlayer)->effectSend);
        if (result != SL_RESULT_SUCCESS)
        {
            LOGE("Couldn't get interface effect send");
        }
    }

    // get the mute/solo interface
    result = (*playerObject)->GetInterface(playerObject, SL_IID_MUTESOLO, &(*audioPlayer)->muteSolo);
    if (result != SL_RESULT_SUCCESS)
    {
        LOGE("Couldn't get interface MuteSolo");
    }

    return codes::SUCCESS;
}

int play(AudioPlayer **audioPlayer) {
    SLPlayItf playerPlay = (*audioPlayer)->playerPlay;
    SLresult result = (*playerPlay)->SetPlayState(playerPlay, SL_PLAYSTATE_PLAYING);
    if (result != SL_RESULT_SUCCESS)
    {
        LOGE("Couldn't set PlayState to Playing");
        return codes::ERROR_SET_PLAYING_STATE_TO_PLAYING;
    }

    return codes::SUCCESS;
}

int pause(AudioPlayer **audioPlayer) {
    SLPlayItf playerPlay = (*audioPlayer)->playerPlay;
    SLresult result = (*playerPlay)->SetPlayState(playerPlay, SL_PLAYSTATE_PAUSED);
    if (result != SL_RESULT_SUCCESS)
    {
        LOGE("Couldn't set PlayState to Paused");
        return codes::ERROR_SET_PLAYING_STATE_TO_PAUSED;
    }

    return codes::SUCCESS;
}

int stop(AudioPlayer **audioPlayer) {
    SLPlayItf playerPlay = (*audioPlayer)->playerPlay;
    SLresult result = (*playerPlay)->SetPlayState(playerPlay, SL_PLAYSTATE_STOPPED);
    if (result != SL_RESULT_SUCCESS)
    {
        LOGE("Couldn't set PlayState to stopped");
        return codes::ERROR_SET_PLAYING_STATE_TO_STOPPED;
    }

    return codes::SUCCESS;
}

int setLooping(AudioPlayer **audioPlayer, bool isLooping)
{
    SLSeekItf playerSeek = (*audioPlayer)->playerSeek;
    SLresult result = (*playerSeek)->SetLoop(playerSeek, (SLboolean) isLooping, 0, SL_TIME_UNKNOWN);
    if (result != SL_RESULT_SUCCESS)
    {
        LOGE("Couldn't set loop for player Seek interface");
        return codes::ERROR_SET_LOOP;
    }

    return codes::SUCCESS;
}