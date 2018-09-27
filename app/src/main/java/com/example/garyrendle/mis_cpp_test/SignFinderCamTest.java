package com.example.garyrendle.mis_cpp_test;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.SurfaceView;
import android.view.WindowManager;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.android.Utils;
import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.imgproc.Imgproc;

import java.io.IOException;

import static org.opencv.core.CvType.CV_32SC4;


public class SignFinderCamTest extends AppCompatActivity implements CameraBridgeViewBase.CvCameraViewListener2 {


    private static final String TAG = "SignFinderPhotoTest";
    private CameraBridgeViewBase cameraBridgeViewBase;


    private Mat signResponse;

    private Mat[] templates = new Mat[4];

    //load template image(s) for matching
    private void load_templates(){
        for (int i = 0; i < templates.length; i++){
            int res = 0;
            switch (i) {
                case 0: res = R.drawable.template30;
                    break;
                case 1: res = R.drawable.template40;
                    break;
                case 2: res = R.drawable.template60;
                    break;
                case 3: res = R.drawable.template_empty;
                    break;
                default:break;
            }

            try {
                //Loading Image to Mat object
                templates[i] = Utils.loadResource(this,res);
                templates[i].convertTo(templates[i], CV_32SC4); //covert to be able to hold negative values

                createTemplateMask(templates[i].getNativeObjAddr());

                Log.d(TAG, "load_templates: template type: = " + templates[i].type());
            } catch (IOException e) {
                e.printStackTrace();
            }

        }
    }


    private BaseLoaderCallback baseLoaderCallback = new BaseLoaderCallback(this) {
        @Override
        public void onManagerConnected(int status) {
            switch (status) {
                case LoaderCallbackInterface.SUCCESS:
                    cameraBridgeViewBase.enableView();

                    load_templates();

                    //init Mats
                    signResponse = new Mat(1280, 960, CvType.CV_8UC1);


                    break;
                default:
                    super.onManagerConnected(status);
                    break;
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setContentView(R.layout.activity_sign_finder);
        cameraBridgeViewBase = (CameraBridgeViewBase) findViewById(R.id.camera_view);
        cameraBridgeViewBase.setVisibility(SurfaceView.VISIBLE);
        cameraBridgeViewBase.setCvCameraViewListener(this);
    }

    @Override
    public void onResume(){
        super.onResume();
        if (!OpenCVLoader.initDebug()) {
            Log.d(TAG, "Internal OpenCV library not found. Using OpenCV Manager for initialization");
            OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_3_4_0, this, baseLoaderCallback);
        } else {
            Log.d(TAG, "OpenCV library found inside package. Using it!");
            baseLoaderCallback.onManagerConnected(LoaderCallbackInterface.SUCCESS);
        }
    }

    @Override
    public void onPause() {
        super.onPause();
        if (cameraBridgeViewBase != null)
            cameraBridgeViewBase.disableView();
    }

    @Override
    public void onCameraViewStarted(int width, int height) {

    }

    @Override
    public void onCameraViewStopped() {

    }

    @Override
    public Mat onCameraFrame(CameraBridgeViewBase.CvCameraViewFrame inputFrame) {

        signResponse = inputFrame.gray();

        //native call
        findSigns(signResponse.getNativeObjAddr(), templates[3].getNativeObjAddr());

        return signResponse;
    }

    public native void findSigns(long matGrey, long template_img);

    public native void createTemplateMask(long template_img);
}

