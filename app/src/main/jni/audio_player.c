#include "audio_player.h"

int createEngine(AudioPlayer *audioPlayer)
{
    SLresult result = slCreateEngine(&audioPlayer->engineObject, 0, NULL, 0, NULL, NULL);
    if (result != SL_RESULT_SUCCESS)
    {
        LOGE("Couldn't create engine object");
        return ERROR_CREATE_ENGINE_OBJECT;
    }
    else
    {
        LOGI("Engine object created");
    }

    result = (*audioPlayer->engineObject)->Realize(audioPlayer->engineObject, SL_BOOLEAN_FALSE); // not async
    if (result != SL_RESULT_SUCCESS)
    {
        LOGE("Couldn't realize engine object");
        return ERROR_REALIZE_ENGINE_OBJECT;
    }
    else
    {
        LOGI("Engine object realized");
    }

    result = (*audioPlayer->engineObject)->GetInterface(audioPlayer->engineObject,
                                                        SL_IID_ENGINE,
                                                        &audioPlayer->engine);
    if (result != SL_RESULT_SUCCESS)
    {
        LOGE("Couldn't GetInterface of engine");
        return ERROR_GET_INTERFACE_ENGINE;
    }
    else
    {
        LOGI("Interface Engine got");
    }

    // create output mix, with environmental reverb specified as non-required interface
    const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req[1] = {SL_BOOLEAN_FALSE};
    result = (*audioPlayer->engine)->CreateOutputMix(audioPlayer->engine,
                                                     &audioPlayer->outputMixObject,
                                                     1, ids, req);
    if (result != SL_RESULT_SUCCESS)
    {
        LOGE("Couldn't create output mix object");
        return ERROR_CREATE_OUTPUT_MIX;
    }
    else
    {
        LOGI("OutputMix object created");
    }

    // realize output mix object
    result = (*audioPlayer->outputMixObject)->Realize(audioPlayer->outputMixObject, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS)
    {
        LOGE("Couldn't realize output mix object");
        return ERROR_REALIZE_OUTPUT_MIX;
    }
    else
    {
        LOGI("OutputMix object realized");
    }

    // get the environmental reverb interface
    // this could fail if the environmental reverb effect is not available,
    // either because the feature is not present, excessive CPU load, or
    // the required MODIFY_AUDIO_SETTINGS permission was not requested and granted
    result = (*audioPlayer->outputMixObject)->GetInterface(audioPlayer->outputMixObject,
                                                           SL_IID_ENVIRONMENTALREVERB,
                                                           &audioPlayer->outputEnvironmentalReverbItf);

    if (result != SL_RESULT_SUCCESS)
    {
        // just log unsuccessful result codes for environmental reverb, as it is optional
        LOGE("Couldn't get interface EnvironmentalReverb");
    }
    else
    {
        LOGI("Interface EnvironmentalReverb got");
    }

    return SUCCESS;
}

int destroyEngine(AudioPlayer *audioPlayer)
{
    if (audioPlayer->playerObject != NULL) {
        (*audioPlayer->playerObject)->Destroy(audioPlayer->playerObject);
        audioPlayer->playerPlay = NULL;
        audioPlayer->playerSeek = NULL;
        audioPlayer->playerVolume = NULL;
        audioPlayer->bufferQueue = NULL;
        audioPlayer->effectSend = NULL;
        audioPlayer->muteSolo = NULL;
        LOGI("Player destroyed");
    }

    if (audioPlayer->outputMixObject != NULL)
    {
        (*audioPlayer->outputMixObject)->Destroy(audioPlayer->outputMixObject);
        audioPlayer->outputMixObject = NULL;
        audioPlayer->outputEnvironmentalReverbItf = NULL;
        LOGI("OutputMix destroyed");
    }

    if (audioPlayer->engineObject != NULL)
    {
        (*audioPlayer->engineObject)->Destroy(audioPlayer->engineObject);
        audioPlayer->engineObject = NULL;
        audioPlayer->engine = NULL;
        LOGI("Engine destroyed");
    }

    return SUCCESS;
}

int initAssetAudioPlayer(AudioPlayer *audioPlayer, AAssetManager *assetManager, const char* fileName)
{
    // use asset manager to open asset by filename
    AAsset* asset = AAssetManager_open(assetManager, fileName, AASSET_MODE_UNKNOWN);

    // the asset might not be found
    if (asset == NULL) {
        LOGE("Couldn't open asset");
        return ERROR_OPEN_ASSET;
    }

    // open asset as file descriptor
    off_t start;
    off_t length;
    int fileDescriptor = AAsset_openFileDescriptor(asset, &start, &length);
    if (fileDescriptor <= 0)
    {
        LOGE("Couldn't open file descriptor");
        return ERROR_OPEN_FILE_DESCRIPTOR;
    }

    //AAsset_close(asset);

    // configure audio source
    SLDataLocator_AndroidFD dataLocatorFileDescriptor = {
            SL_DATALOCATOR_ANDROIDFD,   // SLuint32 locatorType;
            fileDescriptor,             // SLint32 fd;
            start,                      // SLAint64 offset;
            length                      // SLAint64 length;
    };
    SLDataFormat_MIME formatMime = {
            SL_DATAFORMAT_MIME,          // SLuint32 formatType;
            NULL,                        // SLchar *mimeType;
            SL_CONTAINERTYPE_UNSPECIFIED // SLuint32 containerType;
    };
    SLDataSource audioSrc = {
            &dataLocatorFileDescriptor,         // void *pLocator;
            &formatMime                         // void *pFormat;
    };

    /// Configure audio sink
    SLDataLocator_OutputMix outputMix = {
            SL_DATALOCATOR_OUTPUTMIX,       // SLuint32     locatorType;
            audioPlayer->outputMixObject    // SLObjectItf	outputMix;
    };
    SLDataSink audioSink = {
            &outputMix, // void *pLocator;
            NULL        // void *pFormat;
    };

    SLuint32 interfaceNumber = 3;
    // create audio player
    const SLInterfaceID interfacesIds[3] = {
            SL_IID_SEEK,
            SL_IID_MUTESOLO,
            SL_IID_VOLUME
    };
    const SLboolean requiredInterfaces[3] = {
            SL_BOOLEAN_TRUE,
            SL_BOOLEAN_TRUE,
            SL_BOOLEAN_TRUE
    };

    SLresult result = (*audioPlayer->engine)->CreateAudioPlayer(audioPlayer->engine,
                                                                &audioPlayer->playerObject,
                                                                &audioSrc,
                                                                &audioSink,
                                                                interfaceNumber,
                                                                interfacesIds,
                                                                requiredInterfaces);
    if (result != SL_RESULT_SUCCESS)
    {
        LOGE("Couldn't create audio player");
        return ERROR_CREATE_AUDIO_PLAYER;
    }

    // realize the player
    result = (*audioPlayer->playerObject)->Realize(audioPlayer->playerObject, SL_BOOLEAN_FALSE); // not async
    if (result != SL_RESULT_SUCCESS)
    {
        LOGE("Couldn't realize player object");
        return ERROR_REALIZE_AUDIO_PLAYER;
    }

    // get the play interface
    result = (*audioPlayer->playerObject)->GetInterface(audioPlayer->playerObject, SL_IID_PLAY, &audioPlayer->playerPlay);
    if (result != SL_RESULT_SUCCESS)
    {
        LOGE("Couldn't get interface Play");
        return ERROR_GET_INTERFACE_PLAY;
    }

    // get the seek interface
    result = (*audioPlayer->playerObject)->GetInterface(audioPlayer->playerObject, SL_IID_SEEK, &audioPlayer->playerSeek);
    if (result != SL_RESULT_SUCCESS)
    {
        LOGE("Couldn't get interface Seek");
        return ERROR_GET_INTERFACE_SEEK;
    }

    // get the mute/solo interface
    result = (*audioPlayer->playerObject)->GetInterface(audioPlayer->playerObject, SL_IID_MUTESOLO, &audioPlayer->muteSolo);
    if (result != SL_RESULT_SUCCESS)
    {
        LOGE("Couldn't get interface MuteSolo");
        return ERROR_GET_INTERFACE_MUTE_SOLO;
    }

    // get the volume interface
    result = (*audioPlayer->playerObject)->GetInterface(audioPlayer->playerObject, SL_IID_VOLUME, &audioPlayer->playerVolume);
    if (result != SL_RESULT_SUCCESS)
    {
        LOGE("Couldn't get interface Volume");
        return ERROR_GET_INTERFACE_VOLUME;
    }

    // enable whole file looping
    result = (*audioPlayer->playerSeek)->SetLoop(audioPlayer->playerSeek, SL_BOOLEAN_TRUE, 0, SL_TIME_UNKNOWN);
    if (result != SL_RESULT_SUCCESS)
    {
        LOGE("Couldn't set loop");
    }

    return SUCCESS;
}

void onBufferPlayFinished(SLAndroidSimpleBufferQueueItf bufferQueue, void *context)
{
//    AudioPlayer **audioPlayer = (AudioPlayer**) context;
//
//    if ((*audioPlayer)->bu != NULL) {
//        free(player->buffer);
//        player->buffer = NULL;
//    }
//
//    int len = 4096;
//    player->buffer = malloc(len);
//
//    is->audio_callback(context, player->buffer, len);
//    enqueue(&is->audio_player, (int16_t *) player->buffer, len);
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
    const SLInterfaceID ids[4] = {
            SL_IID_BUFFERQUEUE,
            SL_IID_VOLUME,
            SL_IID_EFFECTSEND,
            SL_IID_MUTESOLO
    };
    const SLboolean requiredInterfaces[4] = {
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
        return ERROR_CREATE_AUDIO_PLAYER;
    }

    // realize player
    result = (*playerObject)->Realize(playerObject, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS)
    {
        LOGE("Couldn't realize audio player");
        return ERROR_REALIZE_AUDIO_PLAYER;
    }

    // get play interface
    result = (*playerObject)->GetInterface(playerObject, SL_IID_PLAY, &(*audioPlayer)->playerPlay);
    if (result != SL_RESULT_SUCCESS)
    {
        LOGE("Couldn't get interface Play");
        return ERROR_GET_INTERFACE_PLAY;
    }

    // get buffer queue interface
    SLAndroidSimpleBufferQueueItf bufferQueue = (*audioPlayer)->bufferQueue;
    result = (*playerObject)->GetInterface(playerObject, SL_IID_BUFFERQUEUE, &bufferQueue);
    if (result != SL_RESULT_SUCCESS)
    {
        LOGE("Couldn't get interface BufferQueue");
        return ERROR_GET_INTERFACE_BUFFER_QUEUE;
    }
    // register callback to a buffer queue
    result = (*bufferQueue)->RegisterCallback(bufferQueue, onBufferPlayFinished, NULL);
    if (result != NULL)
    {
        LOGE("Couldn't register callback for BufferQueue");
        return ERROR_REGISTER_BUFFER_QUEUE_CALLBACK;
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

    return SUCCESS;
}

SLuint32 setPlayingState(AudioPlayer *audioPlayer, SLuint32 playingState) {
    return (*audioPlayer->playerPlay)->SetPlayState(audioPlayer->playerPlay, playingState);
}

int play(AudioPlayer *audioPlayer) {
    SLresult result = setPlayingState(audioPlayer, SL_PLAYSTATE_PLAYING);
    if (result != SL_RESULT_SUCCESS)
    {
        LOGE("Couldn't set PlayState to Playing");
        return ERROR_SET_PLAYING_STATE_TO_PLAYING;
    }

    return SUCCESS;
}

int pause(AudioPlayer *audioPlayer) {
    SLresult result = setPlayingState(audioPlayer, SL_PLAYSTATE_PAUSED);
    if (result != SL_RESULT_SUCCESS)
    {
        LOGE("Couldn't set PlayState to Paused");
        return ERROR_SET_PLAYING_STATE_TO_PAUSED;
    }

    return SUCCESS;
}

int stop(AudioPlayer *audioPlayer) {
    SLresult result = setPlayingState(audioPlayer, SL_PLAYSTATE_STOPPED);
    if (result != SL_RESULT_SUCCESS)
    {
        LOGE("Couldn't set PlayState to stopped");
        return ERROR_SET_PLAYING_STATE_TO_STOPPED;
    }

    return SUCCESS;
}

int enqueueBufferData(AudioPlayer *audioPlayer, int16_t *data, int size)
{
    SLAndroidSimpleBufferQueueItf bufferQueue = audioPlayer->bufferQueue;
    SLresult result = (*bufferQueue)->Enqueue(bufferQueue, data, (SLuint32) size);

    if (result != SL_RESULT_SUCCESS)
    {
        LOGE("Couldn't enqueue buffer data");
        return ERROR_ENQUEUE_BUFFER_DATA;
    }

    return SUCCESS;
}

int setLooping(AudioPlayer *audioPlayer, bool isLooping)
{
    SLSeekItf playerSeek = audioPlayer->playerSeek;
    SLresult result = (*playerSeek)->SetLoop(playerSeek, (SLboolean) isLooping, 0, SL_TIME_UNKNOWN);
    if (result != SL_RESULT_SUCCESS)
    {
        LOGE("Couldn't set loop for player Seek interface");
        return ERROR_SET_LOOP;
    }

    return SUCCESS;
}

int getVolume(AudioPlayer *audioPlayer)
{
    SLVolumeItf volumeItf = audioPlayer->playerVolume;
    SLmillibel millibelVolumeLevel;
    SLresult result = (*volumeItf)->GetVolumeLevel(volumeItf, &millibelVolumeLevel);

    if (result != SL_RESULT_SUCCESS)
    {
        LOGE("Couldn't get volume");
        return NO_VALUE;
    }

    return millibelVolumeLevel;
}

int setVolume(AudioPlayer *audioPlayer, float volumeFactor)
{
    SLVolumeItf playerVolume = audioPlayer->playerVolume;

    //get min & max
    SLmillibel minVolume = SL_MILLIBEL_MIN;
    SLmillibel maxVolume = 0;
    (*playerVolume)->GetMaxVolumeLevel(playerVolume, &maxVolume);
    float volumeIncreaseValue = ((float)(maxVolume - minVolume)) * volumeFactor;
    SLmillibel volume = minVolume + (SLmillibel) volumeIncreaseValue;
    SLresult result = (*playerVolume)->SetVolumeLevel(playerVolume, volume);
    if (result != SL_RESULT_SUCCESS)
    {
        LOGE("Couldn't set volume");
        return ERROR_SET_VOLUME;
    }

    return SUCCESS;
}