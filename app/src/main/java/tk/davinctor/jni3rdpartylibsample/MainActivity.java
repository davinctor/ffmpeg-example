package tk.davinctor.jni3rdpartylibsample;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {
    private static final String TAG = "MainActivity";

    private TextView textView;
    private VideoSurfaceView videoSurfaceView;
    private Button btnControl;

    private VideoPlayer videoPlayer;
    private boolean isPlayed;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        textView = (TextView) findViewById(R.id.textView);
        videoSurfaceView = (VideoSurfaceView) findViewById(R.id.videoSurfaceView);
        btnControl = (Button) findViewById(R.id.btnControl);
        btnControl.setOnClickListener(this);
        videoPlayer = new VideoPlayer();
    }

    @Override
    protected void onStart() {
        super.onStart();
    }

    @Override
    protected void onStop() {
        videoPlayer.destroy();
        super.onStop();
    }

    @Override
    public void onClick(View view) {
        isPlayed = !isPlayed;
        if (isPlayed) {
            videoPlayer.init(videoSurfaceView);
            videoPlayer.play();
            btnControl.setText("Pause");
        } else {
            videoPlayer.pause();
            btnControl.setText("Play");
        }
    }
}
