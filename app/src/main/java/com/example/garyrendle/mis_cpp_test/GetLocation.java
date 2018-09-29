package com.example.garyrendle.mis_cpp_test;

import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageManager;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.widget.Toast;

import java.util.Locale;

import static android.content.Context.LOCATION_SERVICE;


public class GetLocation  {

    private static final int PERMISSIONS_REQUEST_ACCESS_FINE_LOCATION = 1;
    private static final float MIN_UPDATE_DIST = 1;
    private static final long MIN_UPDATE_TIME = 3000;



    Context c;
    boolean mLocationPermissionGranted;
    public GetLocation(Context context, boolean mLocationPermissionGranted) {
        this.c = context;
        this.mLocationPermissionGranted = mLocationPermissionGranted;

    }

    //location setup
    public boolean getLocationPermission() {
        if (ContextCompat.checkSelfPermission(c.getApplicationContext(),
                android.Manifest.permission.ACCESS_FINE_LOCATION)
                == PackageManager.PERMISSION_GRANTED) {
            return true;
        } else {
            ActivityCompat.requestPermissions((Activity) c.getApplicationContext(),
                    new String[]{android.Manifest.permission.ACCESS_FINE_LOCATION},
                    PERMISSIONS_REQUEST_ACCESS_FINE_LOCATION);
        }
        return false;
    }

    public void registerForLocationUpdates() {
        LocationManager locationManager;
        try {
            locationManager = (LocationManager) c.getSystemService(LOCATION_SERVICE);
            locationManager.requestLocationUpdates(LocationManager.GPS_PROVIDER,
                    MIN_UPDATE_TIME, MIN_UPDATE_DIST, (LocationListener) c);
        }
        catch(SecurityException e){
            Toast.makeText(c.getApplicationContext(), "ERROR: " + e.getMessage(), Toast.LENGTH_LONG).show();
            Log.e("TAG","Exception: " + e.getMessage());
        }
    }






}
