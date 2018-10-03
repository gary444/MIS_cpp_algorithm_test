package com.example.garyrendle.mis_cpp_test;

import android.location.Location;
import android.location.LocationListener;
import android.os.Handler;
import android.speech.tts.TextToSpeech;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import java.util.Locale;


public class MainActivity extends AppCompatActivity implements LocationListener, TextToSpeech.OnInitListener {

    private static final String TAG = "MainActivity";
    private double latitude;
    private double longitude;
    private int maxSpeed;
    private GetContentTask asyncTask;
    private String url;
    public boolean mLocationPermissionGranted;

    private TextToSpeechManager speech_engine;
    boolean speech_engine_ready = false;

    private double last_known_speed = 0.0;
    private ImageView ivSign;
    private TextView tvMaxSpeed;
    private TextView tvCurrentSpeed;
    private TextView tvCurrentStr;

    // private SignFinderBackground signFinderBG;
    private GetLocation getloc;


    // load c++ code library
    static {
        System.loadLibrary("sign-finder-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        ivSign = findViewById(R.id.ivSignBackground);
        ivSign.setVisibility(View.INVISIBLE);

        tvMaxSpeed = findViewById(R.id.tvMaxSpeed);
        tvCurrentSpeed = findViewById(R.id.tvCurrentSpeed);
        tvCurrentStr = findViewById(R.id.tvCurrentStr);

        //text to speech engine
        speech_engine = new TextToSpeechManager(this, this);

        //get current location
        getloc = new GetLocation(this, mLocationPermissionGranted);
        mLocationPermissionGranted = getloc.getLocationPermission();
        Log.d("TAG", "PER1: " + mLocationPermissionGranted);

        if (mLocationPermissionGranted)
            getloc.registerForLocationUpdates();

    }

    @Override
    public void onLocationChanged(final Location location) {
        latitude = location.getLatitude();
        longitude = location.getLongitude();
        double radius = 10.0;
        //url to request speed limit from a given location
        url = "https://overpass-api.de/api/interpreter?data=[out:json];" +
                "way[maxspeed](around:" + radius + "," + latitude + "," + longitude + ");" +
                "out%20tags;";

        //get current speed
        last_known_speed = toKmh(location.getSpeed());
        tvCurrentSpeed.setText(String.format(Locale.UK, "%.1f KM/H", last_known_speed));


        //get the maxSpeed
        asyncTask = (GetContentTask) new GetContentTask(new GetContentTask.AsyncResponse() {

            @Override
            public void processFinish(RoadInfo output) {
                Log.d("TAG", "onPostExecute: " + output);
                //If maxspeed is -100 -> error parsing json
                //If maxspeed is -2 -> error calculating new value
                // maxSpeed = output;

                //pronounce maxspeed
                if (output.getMaxSpeed() == -2) {
                    //If maxspeed is -2 -> error calculating new value
                    // if (speech_engine_ready) {speech_engine.speechErrCalc();};
                    ivSign.setVisibility(View.INVISIBLE);
                    tvMaxSpeed.setText("");
                } else if (maxSpeed == -100) {
                    //If maxspeed is -100 -> error parsing json
                    // if (speech_engine_ready) {speech_engine.speechErrJSON();};
                    ivSign.setVisibility(View.INVISIBLE);
                    tvMaxSpeed.setText("");
                } else {
                    //max allowed speed
                    maxSpeed = output.getMaxSpeed();

                    if (maxSpeed < last_known_speed) {
                        //notification that max speed is exceeded
                        if (speech_engine_ready) {
                            speech_engine.speechSpeedExceeded();
                        }
                        ;
                        // sign is flashing/blinking
                        blinkSpeedSign();
                    } else {
                        ivSign.setVisibility(View.VISIBLE);
                        tvMaxSpeed.setText(String.format(Locale.UK, "%d", maxSpeed));

                    }

                    //if (speech_engine_ready) {speech_engine.speech(last_known_speed, maxSpeed);};
                }

                tvCurrentStr.setText(output.getRoadName());
                Log.d("TAG", "INFO: " + latitude + " " + longitude + " " + maxSpeed);

            }
        }, this).execute(url);


    }

    private void blinkSpeedSign() {
        ivSign.setVisibility(View.VISIBLE);


        final Handler handler = new Handler();
        Runnable runnableCode = new Runnable() {
            int counter = 0;

            @Override
            public void run() {
                // Do something here on the main thread
                Log.d("TAG", "Called on main thread: " + counter);

                if (ivSign.getVisibility() == View.VISIBLE) {
                    ivSign.setVisibility(View.INVISIBLE);
                    tvMaxSpeed.setText("");
                } else {
                    ivSign.setVisibility(View.VISIBLE);
                    tvMaxSpeed.setText(String.format(Locale.UK, "%d", maxSpeed));

                }

                counter++;

                if (counter < 6) {
                    handler.postDelayed(this, 200);
                }

            }
        };

        handler.post(runnableCode);


    }

    @Override
    public void onStatusChanged(String s, int i, Bundle bundle) {

    }

    @Override
    public void onProviderEnabled(String s) {

    }

    @Override
    public void onProviderDisabled(String s) {

    }

    //helper - conversion function, m/s to km/h
    private float toKmh(float in_metres_per_sec) {
        return in_metres_per_sec * 60 * 60 / 1000;
    }


    //text to speech callback
    @Override
    public void onInit(int status) {
        if (status == TextToSpeech.SUCCESS) {
            Log.d("Speech", "Success!");
            speech_engine_ready = true;
        }
    }

    //activity lifecycle callbacks-----------------------------------
    @Override
    protected void onResume() {
        super.onResume();
        //CameraBridgeViewBase cb = findViewById(R.id.dummy_camera_view);
        //signFinderBG.startFindingSigns(cb);
    }

    @Override
    protected void onPause() {
        super.onPause();

        // signFinderBG.stopFindingSigns();
    }

    @Override
    protected void onDestroy() {

        getloc.stopLocationUpdates();
        speech_engine.destroy();
        super.onDestroy();
    }
}
