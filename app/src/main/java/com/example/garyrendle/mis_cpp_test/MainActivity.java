package com.example.garyrendle.mis_cpp_test;

import android.content.Intent;
import android.location.Location;
import android.location.LocationListener;
import android.speech.tts.TextToSpeech;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import org.opencv.android.CameraBridgeViewBase;

import java.util.Locale;

import static android.widget.Toast.LENGTH_SHORT;


public class MainActivity extends AppCompatActivity implements LocationListener, TextToSpeech.OnInitListener {

    private static final String TAG = "MainActivity";
    private double latitude;
    private double longitude;
    private int maxSpeed;
    private GetContentTask asyncTask;
    private String url;
    private TextToSpeech t1;
    public boolean mLocationPermissionGranted;
    TextToSpeech engine;
    private double last_known_speed = 0.0;
    private ImageView ivSign;
    private TextView tvMaxSpeed;
    private TextView tvCurrentSpeed;

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

        ivSign = findViewById(R.id.ivSignBackground);
        ivSign.setVisibility(View.INVISIBLE);

        tvMaxSpeed = findViewById(R.id.tvMaxSpeed);
        tvCurrentSpeed = findViewById(R.id.tvCurrentSpeed);

        //text to speech engine
        engine = new TextToSpeech(this, this);

        //get current location
        GetLocation getloc = new GetLocation(this,mLocationPermissionGranted );
        mLocationPermissionGranted = getloc.getLocationPermission();
        Log.d("TAG", "PER1: " + mLocationPermissionGranted);

        if(mLocationPermissionGranted)
            getloc.registerForLocationUpdates();
        Log.d("TAG", "PER: " + mLocationPermissionGranted);


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
    public void onLocationChanged(Location location) {
        latitude = location.getLatitude();
        longitude = location.getLongitude();
        double radius = 50.0;
        //url to request speed limit from a given location
        url = "https://overpass-api.de/api/interpreter?data=[out:json];" +
                "way[maxspeed](around:" + radius + "," + latitude + "," + longitude + ");" +
                "out%20tags;";
        //get the maxSpeed
        asyncTask = (GetContentTask) new GetContentTask(new GetContentTask.AsyncResponse(){

            @Override
            public void processFinish(Integer output) {
                Log.d("TAG", "onPostExecute: " + output);
                //If maxspeed is -100 -> error parsing json
                //If maxspeed is -2 -> error calculating new value
                maxSpeed = output;
            }
        }).execute(url);

        //pronounce maxspeed
        switch (maxSpeed){
            case 0:
                speechWelcome();
                ivSign.setVisibility(View.INVISIBLE);
                tvMaxSpeed.setText("");
                break;
            case -2:
                //If maxspeed is -2 -> error calculating new value
                speechErrCalc();
                ivSign.setVisibility(View.INVISIBLE);
                tvMaxSpeed.setText("");
                break;
            case -100:
                //If maxspeed is -100 -> error parsing json
                ivSign.setVisibility(View.INVISIBLE);
                tvMaxSpeed.setText("");
                speechErrJSON();
            default:
                ivSign.setVisibility(View.VISIBLE);
                tvMaxSpeed.setText(String.format("%d", maxSpeed));
                speech();
                break;
        }

        //get current speed
        last_known_speed = toKmh(location.getSpeed());
        tvCurrentSpeed.setText(String.format("%.1f KM/H", last_known_speed));

        //
        Log.d("TAG", "INFO: " + latitude + " " + longitude + " " + maxSpeed);
        Toast.makeText(this, "Location :" + latitude + " " +longitude
                + " " + maxSpeed + " " + String.format("%.1f", last_known_speed),
                Toast.LENGTH_LONG).show();


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
    private float toKmh(float in_metres_per_sec){
        return in_metres_per_sec * 60 * 60 / 1000;
    }

    // text to speech
    private void speechWelcome() {
        engine.speak("Initialization",
                TextToSpeech.QUEUE_FLUSH, null, null);
    }

    private void speechErrCalc() {
        engine.speak("No Speed Limit Detected",
                TextToSpeech.QUEUE_FLUSH, null, null);
    }

    private void speechErrJSON() {
        engine.speak("Error parsing Jason",
                TextToSpeech.QUEUE_FLUSH, null, null);
    }

    private void speech() {
        engine.speak("current" + String.format("%.1f", last_known_speed) + "maximum" + String.format("%d", maxSpeed),
                TextToSpeech.QUEUE_FLUSH, null, null);
    }
    //initialize tts listener
    @Override
    public void onInit(int status) {
        Log.d("TAG", "OnInit - Status ["+status+"]");

        if (status == TextToSpeech.SUCCESS) {
            Log.d("Speech", "Success!");
            engine.setLanguage(Locale.UK);
        }
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
