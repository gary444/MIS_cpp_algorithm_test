package com.example.garyrendle.mis_cpp_test;

import android.app.Activity;
import android.os.AsyncTask;
import android.util.Log;

import org.json.JSONArray;
import org.json.JSONObject;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;


//ref from: https://androidkennel.org/android-networking-tutorial-with-asynctask/
public class GetContentTask extends AsyncTask<String, Void, RoadInfo> {

    private static final String TAG = "GetContentTask";
    RoadInfo roadInfo;

    private Activity activity;

    public GetContentTask(MainActivity activity){
        this.activity = activity;
    }

    @Override
    protected RoadInfo doInBackground(String... strings) {

        // insert protocol prefix if missing
        String urlString = strings[0];
        if (!urlString.startsWith("https://")){
            urlString = "https://" + urlString;
        }

        String inputString;
        try {
            URL url = new URL(urlString);
            HttpURLConnection urlConnection = (HttpURLConnection) url.openConnection();

            InputStream stream = new BufferedInputStream(urlConnection.getInputStream());
            BufferedReader bufferedReader = new BufferedReader(new InputStreamReader(stream));
            StringBuilder builder = new StringBuilder();

            while ((inputString = bufferedReader.readLine()) != null) {
                builder.append(inputString);
            }

            urlConnection.disconnect();
//


            String JSON_string = builder.toString();
            String road_name = "";

            //get max speed from JSON
            int maxspeed = -2;


            JSONObject jsonObj = new JSONObject(JSON_string);
            JSONArray elements = jsonObj.getJSONArray("elements");
            if (elements.length() > 0){
                JSONObject e = elements.getJSONObject(0);
                JSONObject tags = e.getJSONObject("tags");
                String maxspeed_s = tags.getString("maxspeed");
                road_name = tags.getString("name");
                maxspeed = Integer.parseInt(maxspeed_s);
            }

//            final String r_name = road_name;


//            activity.runOnUiThread(new Runnable() {
//                public void run() {
//                    Toast.makeText(activity, "road name: " + r_name, Toast.LENGTH_SHORT).show();
//                }
//            });

            return new RoadInfo(maxspeed, road_name);

        } catch (Exception  e) {
            //print error

            final String err = "Error: " + e.getMessage();
            Log.d(TAG, "doInBackground: " + err);

            //show toast message on ui thread
            //ref: https://stackoverflow.com/questions/3134683/android-toast-in-a-thread
//            activity.runOnUiThread(new Runnable() {
//                public void run() {
//                    Toast.makeText(activity, err, Toast.LENGTH_SHORT).show();
//                }
//            });

            roadInfo.setMaxSpeed(-100);
            roadInfo.setRoadName("");
            return roadInfo;
        }
    }
    //interface to send value back to main activity
    public interface AsyncResponse {
        void processFinish(RoadInfo output);
    }

    public AsyncResponse delegate = null;

    public GetContentTask(AsyncResponse delegate, Activity activity){

        this.delegate = delegate;
        this.activity = activity;
    }
    @Override
    protected void onPostExecute(RoadInfo input) {
        //"input" is sent to main activity
        delegate.processFinish(input);
        activity = null;
    }
}
