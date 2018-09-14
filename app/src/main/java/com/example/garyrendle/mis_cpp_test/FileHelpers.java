package com.example.garyrendle.mis_cpp_test;

import android.content.Context;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class FileHelpers {

    private static final String TAG = "FileHelpers";

    public static String initAssetFile(String filename, Context context)  {
        File file = new File(context.getFilesDir(), filename);
        if (!file.exists()) try {
            InputStream is = context.getAssets().open(filename);
            OutputStream os = new FileOutputStream(file);
            byte[] data = new byte[is.available()];
            is.read(data); os.write(data); is.close(); os.close();
            Log.d(TAG,"prepared local file: "+filename);
        }
        catch (IOException e) {
            e.printStackTrace();
            Log.d(TAG, "initAssetFile: file not found: " + filename);
        }
        return file.getAbsolutePath();
    }
}
