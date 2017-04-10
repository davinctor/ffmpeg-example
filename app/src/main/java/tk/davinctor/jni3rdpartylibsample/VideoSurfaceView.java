package tk.davinctor.jni3rdpartylibsample;

/**
 * Created by davinctor on 10.04.17.
 */
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.os.Build;
import android.support.annotation.RequiresApi;
import android.util.AttributeSet;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import java.util.Locale;

/**
 * 04.08.16
 */
public class VideoSurfaceView extends SurfaceView implements SurfaceHolder.Callback {

    private static final String TAG = "VideoSurfaceView";

    private Paint paint;
    private float scaleFactor;
    private int height;
    private int width;
    private int left;
    private int top;

    public VideoSurfaceView(Context context) {
        super(context);
        init();
    }

    public VideoSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    public VideoSurfaceView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init();
    }

    @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
    public VideoSurfaceView(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
        init();
    }

    private void init() {
        getHolder().addCallback(this);
        paint = new Paint();
    }

    public int getSurfaceVideoWidth() {
        return width;
    }

    public int getSurfaceVideoHeight() {
        return height;
    }

    public void setResolution(int width, int height) {
        float fWidth = (float) width;
        float fHeight = (float) height;
        int viewWidth = getWidth();
        int viewHeight = getHeight();
        float scaleFactor = 1.0f;

        if (viewWidth > 0 && viewHeight > 0) {
            if (fWidth / viewWidth > height / viewHeight) {
                // fit width
                scaleFactor = fWidth / viewWidth;
                this.left = 0;
                this.top = (int) ((viewHeight - fHeight / scaleFactor) / 2f);
            } else {
                // fit height
                scaleFactor = fHeight / viewHeight;
                this.left = (int) ((viewWidth - fWidth / scaleFactor) / 2f);
                this.top = 0;
            }
        }

        this.scaleFactor = scaleFactor;
        this.width = (int) (fWidth / scaleFactor);
        this.height = (int) (fHeight / scaleFactor);
        Log.d(TAG,
                String.format(Locale.US, "setResolution: left %1$d, top %2$d, scaleFactor %3$f, width %4$d, height %5$d",
                        left, top, scaleFactor, width, height));
    }

    public void drawFrame(Bitmap frameBitmap) {
        if (frameBitmap == null) {
            Log.w(TAG, "drawFrame: bitmap is null");
            return;
        }
        SurfaceHolder holder = getHolder();
        if (holder.getSurface().isValid()) {
            Canvas canvas = holder.lockCanvas();
            canvas.drawBitmap(frameBitmap, left, top, paint);
            holder.unlockCanvasAndPost(canvas);
        }
    }

    @Override
    public void surfaceCreated(SurfaceHolder surfaceHolder) {
        Log.d(TAG, "surfaceCreated()");
    }

    @Override
    public void surfaceChanged(SurfaceHolder surfaceHolder, int i, int width, int height) {
        Log.d(TAG, String.format(Locale.US, "surfaceChanged: width %1$d height %2$d", width, height));
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder surfaceHolder) {
        Log.d(TAG, "surfaceDestroyed()");
    }
}
