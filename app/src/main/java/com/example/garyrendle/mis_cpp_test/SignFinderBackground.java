package com.example.garyrendle.mis_cpp_test;

import android.content.Context;
import android.util.Log;
import android.view.SurfaceView;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.android.Utils;
import org.opencv.core.Mat;
import org.opencv.imgproc.Imgproc;

import java.io.IOException;

import static org.opencv.core.CvType.CV_32SC4;

public class SignFinderBackground implements CameraBridgeViewBase.CvCameraViewListener2 {

    private static final String TAG = "SignFinderBackground";

    //define listener class
    public interface SignFinderBackgroundListener {
        void signFound(int speed);
    }

    private SignFinderBackgroundListener listener;
    private CameraBridgeViewBase cameraBridgeViewBase;
    private Context context;

    private Mat template_img;
    private Mat test_img;
    private int frameCount = 0;
    private int img_index = 1;

    SignFinderBackground(Context context){
        this.listener = null;
        this.context = context;

        if (!OpenCVLoader.initDebug()) {
            Log.d(TAG, "Internal OpenCV library not found. Using OpenCV Manager for initialization");
            OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_3_4_0, context, baseLoaderCallback);
        } else {
            Log.d(TAG, "OpenCV library found inside package. Using it!");
            baseLoaderCallback.onManagerConnected(LoaderCallbackInterface.SUCCESS);
        }
    }

    private BaseLoaderCallback baseLoaderCallback = new BaseLoaderCallback(context) {
        @Override
        public void onManagerConnected(int status) {
            Log.d(TAG, "onManagerConnected: called");

            switch (status) {
                case LoaderCallbackInterface.SUCCESS:
                    Log.d(TAG, "onManagerConnected: base loader success");

                    load_template();
                    test_img = load_test_image();

                    break;
                default:
                    super.onManagerConnected(status);
                    break;
            }
        }


    };

    public void setListener(final SignFinderBackgroundListener listener){
        this.listener = listener;
    }

    public void startFindingSigns(CameraBridgeViewBase parent_cameraBridgeViewBase){

        cameraBridgeViewBase = parent_cameraBridgeViewBase;
        cameraBridgeViewBase.enableView();
        cameraBridgeViewBase.setVisibility(SurfaceView.VISIBLE);
        cameraBridgeViewBase.setCvCameraViewListener(this);
    }

    public void stopFindingSigns(){
        if (cameraBridgeViewBase != null)
            cameraBridgeViewBase.disableView();
    }

    @Override
    public void onCameraViewStarted(int width, int height) {
        Log.d(TAG, "onCameraViewStarted: camera view started");
    }

    @Override
    public void onCameraViewStopped() {
        Log.d(TAG, "onCameraViewStopped: camera view stopped");
    }

    @Override
    public Mat onCameraFrame(CameraBridgeViewBase.CvCameraViewFrame inputFrame) {

        Mat sf_input = test_img.clone();

        //native call
        //int response_val = findSigns(sf_input.getNativeObjAddr(), template_img.getNativeObjAddr());
            int response_val = 0;
        Log.d(TAG, "onCameraFrame: response received = " + response_val);

        frameCount++;
        if (frameCount > 20){
            frameCount = 0;
            test_img = load_test_image();

            listener.signFound(response_val);
        }

        return null;
//        return inputFrame.gray();
    }

    //loads a series of images to Mat object
    private Mat load_test_image() {
        int res = 0;
        switch ((img_index % 4) + 1){
            case 1: res = R.drawable.test1;
                break;
            case 2: res = R.drawable.test2;
                break;
            case 3: res = R.drawable.test3;
                break;
            case 4: res = R.drawable.test4;
                break;
            default:break;
        }

        Mat img = null;
        try {
            //Loading Image to Mat object
            img = Utils.loadResource(context,res);
            Imgproc.cvtColor(img, img, Imgproc.COLOR_RGB2GRAY);
            Log.d(TAG, "load_test_image: loaded successfully");
            img_index++;
        } catch (IOException e) {
            e.printStackTrace();
        }

        return img;
    }
    //load template image(s) for matching
    private void load_template(){
        int res = R.drawable.template_empty;
        try {
            //Loading Image to Mat object
            template_img = Utils.loadResource(context,res);
            template_img.convertTo(template_img, CV_32SC4); //covert to be able to hold negative values

            //createTemplateMask(template_img.getNativeObjAddr());

            Log.d(TAG, "load_templates: template type: = " + template_img.type());
        } catch (IOException e) {
            e.printStackTrace();
        }
    }


    //native calls
//    public native int findSigns(long matGrey, long template_img);
//    public native void createTemplateMask(long template_img);
}
