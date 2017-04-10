package tk.davinctor.jni3rdpartylibsample;

/**
 * Created by davinctor on 10.04.17.
 */
import android.graphics.Bitmap;
import android.support.annotation.IntDef;
import android.util.Log;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.ref.WeakReference;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;

/**
 * 04.08.16
 */
public class VideoPlayer {
    private static final String TAG = "VideoPlayer";
    private static final int NO_VALUE = -1;

    private ExecutorService exec;

    private WeakReference<VideoSurfaceView> surfaceViewReference;
    private Bitmap frameBitmap;

    private boolean isInited = false;
    private int width;
    private int height;
    private int duration;
    private volatile int currentFrame;
    @Status
    private volatile int currentStatus;

    private Future<?> currentPlayingTask;

    public void init(VideoSurfaceView videoSurfaceView) {
        int resultCode = FFmpegWrapper.init("/sdcard/video2.mp4");
        if (resultCode != FFmpegWrapper.ResultCode.SUCCESS) {
            Log.w(TAG, String.format("FFmpegWrapper.init() - code '%d' : message '%s'",
                    resultCode,
                    FFmpegWrapper.getRationaleMessageByResultCode(resultCode)));
            return;
        }
        surfaceViewReference = new WeakReference<>(videoSurfaceView);
        exec = Executors.newSingleThreadExecutor();
        int[] resolution = FFmpegWrapper.getVideoResolution();
        videoSurfaceView.setResolution(resolution[0], resolution[1]);
        width = videoSurfaceView.getSurfaceVideoWidth();
        height = videoSurfaceView.getSurfaceVideoHeight();
        duration = (int) FFmpegWrapper.getVideoDuration();
        currentStatus = Status.STOP;
        frameBitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
        FFmpegWrapper.prepareDisplay(frameBitmap, width, height);
        isInited = true;
    }

    public void destroy() {
        isInited = false;
        exec.shutdownNow();
        exec = null;
        surfaceViewReference.enqueue();
        surfaceViewReference = null;
        currentPlayingTask = null;
        FFmpegWrapper.finish();
        currentStatus = Status.STOP;
        width = NO_VALUE;
        height = NO_VALUE;
        currentFrame = NO_VALUE;
    }

    public void play() {
        if (!isInited) {
            throw new IllegalStateException("Before all must be init() method call");
        }
        if (currentStatus == Status.PLAY) {
            Log.w(TAG, "play: video already played");
            return;
        }
        currentStatus = Status.PLAY;
        if (currentPlayingTask == null || currentPlayingTask.isCancelled() || currentPlayingTask.isDone()) {
            currentPlayingTask = exec.submit(new Runnable() {
                @Override
                public void run() {
                    //noinspection InfiniteLoopStatement
                    for (;;) {
                        if (currentStatus == Status.PLAY) {
                            currentFrame = FFmpegWrapper.getVideoFrame();
                            VideoSurfaceView surfaceReference = surfaceViewReference.get();
                            if (surfaceReference != null) {
                                surfaceReference.drawFrame(frameBitmap);
                            }
                        }
                    }
                }
            });
        }
    }

    public void pause() {
        currentStatus = Status.PAUSE;
    }

    public void stop() {
        currentStatus = Status.STOP;
        currentPlayingTask.cancel(true);
        currentPlayingTask = null;
        currentFrame = 0;
    }

    @IntDef({Status.STOP, Status.PLAY, Status.PAUSE})
    @Retention(RetentionPolicy.SOURCE)
    private @interface Status {
        int STOP = 0;
        int PLAY = 1;
        int PAUSE = 2;
    }

}
