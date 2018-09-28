package com.example.garyrendle.mis_cpp_test;

import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;

import org.opencv.android.CameraBridgeViewBase;


public class MainActivity extends AppCompatActivity {

    private static final String TAG = "MainActivity";

    private SignFinderBackground signFinderBG;


    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
        System.loadLibrary("sign-finder-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);


        String url = "https://overpass-api.de/api/interpreter?data=[out:json];way[maxspeed](around:50.0,50.9896629,11.3221793);out%20tags;";
        new GetContentTask(this).execute(url);


        // Button to call Sign Finder Activity
        Button signFinderPhotoTest = findViewById(R.id.signFinder);
        signFinderPhotoTest.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent i = new Intent(getApplicationContext(),SignFinderPhotoTest.class);
                startActivity(i);
            }
        });

        // Button to call Sign Finder Activity
        Button signFinderCamTest = findViewById(R.id.signFinderCamTest);
        signFinderCamTest.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent i = new Intent(getApplicationContext(),SignFinderCamTest.class);
                startActivity(i);
            }
        });


        //creates sign finder that processes camera images in a background task
        signFinderBG = new SignFinderBackground(getApplicationContext());
        signFinderBG.setListener(new SignFinderBackground.SignFinderBackgroundListener() {
            @Override
            public void signFound(int speed) {
                Log.d(TAG, "main activity received callback from sign finder: " + speed);
            }
        });

    }

    @Override
    protected void onResume() {
        super.onResume();
        CameraBridgeViewBase cb = findViewById(R.id.dummy_camera_view);
        signFinderBG.startFindingSigns(cb);
    }

    @Override
    protected void onPause() {
        super.onPause();

        signFinderBG.stopFindingSigns();
    }


}
