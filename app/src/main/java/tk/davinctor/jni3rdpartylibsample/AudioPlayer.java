package tk.davinctor.jni3rdpartylibsample;

import android.content.res.AssetManager;

/**
 * 08.08.16
 */
public class AudioPlayer {

    public static native void createEngine();
    public static native void destoryEngine();
    public static native boolean createAssetAudioPlayer(AssetManager assetManager, String filename);
    public static native void setPlayingAssetAudioPlayer(boolean isPlaying);
}
