package com.example.garyrendle.mis_cpp_test;

import android.os.AsyncTask;
import android.util.Log;
import android.widget.Toast;

import org.json.JSONArray;
import org.json.JSONObject;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;


//ref from: https://androidkennel.org/android-networking-tutorial-with-asynctask/
public class GetContentTask extends AsyncTask<String, Void, Integer> {

    private static final String TAG = "GetContentTask";
//
//    private TextView textView;
    private MainActivity activity;

    public GetContentTask(MainActivity activity){
//        this.textView = textView;
        this.activity = activity;
    }

    @Override
    protected Integer doInBackground(String... strings) {

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

            activity.runOnUiThread(new Runnable() {
                public void run() {
                    Toast.makeText(activity, "request success", Toast.LENGTH_SHORT).show();
                }
            });

            String JSON_string = builder.toString();

            //get max speed from JSON
            int maxspeed = -1;
            JSONObject jsonObj = new JSONObject(JSON_string);
            JSONArray elements = jsonObj.getJSONArray("elements");
            if (elements.length() > 0){
                JSONObject e = elements.getJSONObject(0);
                JSONObject tags = e.getJSONObject("tags");
                String maxspeed_s = tags.getString("maxspeed");
                maxspeed = Integer.parseInt(maxspeed_s);
            }

            return maxspeed;

        } catch (Exception  e) {
            //print error

            final String err = "Error: " + e.getMessage();

            //show toast message on ui thread
            //ref: https://stackoverflow.com/questions/3134683/android-toast-in-a-thread
            activity.runOnUiThread(new Runnable() {
                public void run() {
                    Toast.makeText(activity, err, Toast.LENGTH_SHORT).show();
                }
            });

            return -1;
        }
    }

    @Override
    protected void onPostExecute(Integer input) {
        Log.d(TAG, "onPostExecute: " + input);

        Toast.makeText(activity, "max speed = " + input, Toast.LENGTH_SHORT).show();
    }
}
