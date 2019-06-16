package com.darren.music.player;

import android.os.Bundle;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;

import com.darren.media.DarrenPlayer;
import com.darren.media.listener.MediaErrorListener;

import java.io.File;

public class MainActivity extends AppCompatActivity {
    File mMusicFile = new File(Environment.getExternalStorageDirectory(), "input1.mp3");
    // Used to load the 'native-lib' library on application startup.
    private DarrenPlayer mPlayer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Log.e("TAG", "file is exist: " + mMusicFile.exists());

        mPlayer = new DarrenPlayer();
        mPlayer.setDataSource(mMusicFile.getAbsolutePath());

        mPlayer.setOnErrorListener(new MediaErrorListener() {
            @Override
            public void onError(int code, String msg) {
                Log.e("TAG", "error code: " + code);
                Log.e("TAG", "error msg: " + msg);
                // Java 的逻辑代码
            }
        });

        mPlayer.play();
    }
}
