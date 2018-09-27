package com.example.garyrendle.mis_cpp_test;

import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import static android.widget.Toast.LENGTH_SHORT;


public class MainActivity extends AppCompatActivity {

    private static final String TAG = "MainActivity";


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
        //new GetContentTask(this).execute(url);

        GetContentTask asyncTask = (GetContentTask) new GetContentTask(new GetContentTask.AsyncResponse(){

            @Override
            public void processFinish(Integer output) {
                Log.d(TAG, "onPostExecute: " + output);

                //If maxspeed is 100 -> error parsing json
                //If maxspeed is -2 -> error calculating new value
                Toast.makeText(getApplicationContext(), "max speed = " + output, LENGTH_SHORT).show();

            }
        }).execute(url);

        // Button to call Sign Finder Activity
        Button signFinder = (Button)findViewById(R.id.signFinder);
        signFinder.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent i = new Intent(getApplicationContext(),SignFinder.class);
                startActivity(i);
            }
        });
    }

}
