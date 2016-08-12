package tk.davinctor.jni3rdpartylibsample;

import android.content.res.AssetManager;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {
    private static final String TAG = "MainActivity";

    static {
        System.loadLibrary("jni_lib");
    }

    private TextView textView;
    private VideoSurfaceView videoSurfaceView;
    private Button btnPlay;
    private Button btnStop;
    private Button btnVolumePlus;
    private Button btnVolumeMinus;

    private VideoPlayer videoPlayer;
    private boolean isPlayed;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        textView = (TextView) findViewById(R.id.textView);
        videoSurfaceView = (VideoSurfaceView) findViewById(R.id.videoSurfaceView);

        btnPlay = (Button) findViewById(R.id.btnControlPlay);
        btnPlay.setOnClickListener(this);
        btnStop = (Button) findViewById(R.id.btnControlStop);
        btnStop.setOnClickListener(this);
        btnVolumePlus = (Button) findViewById(R.id.btnVolumePlus);
        btnVolumePlus.setOnClickListener(this);
        btnVolumeMinus = (Button) findViewById(R.id.btnVolumeMinus);
        btnVolumeMinus.setOnClickListener(this);

        videoPlayer = new VideoPlayer();
        createEngine();
        createAssetAudioPlayer(getAssets(), "background.mp3");
    }

    @Override
    protected void onResume() {
        super.onResume();
        videoPlayer.init(videoSurfaceView);
    }

    @Override
    protected void onStop() {
        videoPlayer.destroy();
        super.onStop();
    }

    @Override
    protected void onDestroy() {
        destroyEngine();
        super.onDestroy();
    }

    @Override
    public void onClick(View view) {
        /*isPlayed = !isPlayed;
        if (isPlayed) {
            videoPlayer.init(videoSurfaceView);
            videoPlayer.play();
            btnControl.setText("Pause");
        } else {
            videoPlayer.pause();
            btnControl.setText("Play");
        }*/
        switch (view.getId()) {
            case R.id.btnControlPlay:
                play();
                break;
            case R.id.btnControlStop:
                pause();
                break;
            case R.id.btnVolumePlus:
                volumePlus();
                break;
            case R.id.btnVolumeMinus:
                volumeMinus();
                break;
        }
    }

    public static native void createEngine();
    public static native void createAssetAudioPlayer(AssetManager assetManager, String fileName);
    public static native void destroyEngine();

    public static native void play();
    public static native void pause();

    public static native void volumePlus();
    public static native void volumeMinus();
}
