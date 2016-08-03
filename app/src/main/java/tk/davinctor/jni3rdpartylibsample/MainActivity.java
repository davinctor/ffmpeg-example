package tk.davinctor.jni3rdpartylibsample;

import android.annotation.SuppressLint;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {
    private static final String TAG = "MainActivity";

    private TextView textView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        int resultCode = FFmpegWrapper.init("/sdcard/video2.mp4");
        Log.i(TAG, String.format("FFmpegWrapper.init() - code '%d' : message '%s'",
                resultCode,
                FFmpegWrapper.getRationaleMessageByResultCode(resultCode)));
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        textView = (TextView) findViewById(R.id.textView);
    }

    @SuppressLint("DefaultLocale")
    @Override
    protected void onStart() {
        super.onStart();
        int[] resolution = FFmpegWrapper.getVideoResolution();
        textView.append(String.format("\n Video resolution: width %d, height %d.", resolution[0],resolution[1]));
        int duration = (int) FFmpegWrapper.getVideoDuration();
        textView.append(String.format("\n Video duration: %d seconds.", duration));
        int frameRate = (int) FFmpegWrapper.getVideoFrameRate();
        textView.append(String.format("\n Video frame rate: %d fps.", frameRate));
    }

    @Override
    protected void onDestroy() {
        FFmpegWrapper.finish();
        super.onDestroy();
    }
}
