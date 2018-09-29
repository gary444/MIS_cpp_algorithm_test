package com.example.garyrendle.mis_cpp_test;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
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


public class SignFinderPhotoTest extends AppCompatActivity implements CameraBridgeViewBase.CvCameraViewListener2 {


    private static final String TAG = "SignFinderPhotoTest";
    private CameraBridgeViewBase cameraBridgeViewBase;


    private Mat signResponse;

    private Mat test_img;
    private int img_index = 1;
    private int frameCount = 0;

    private Mat template_img;
    private Mat[] number_templates = new Mat[7];
    private long[] tempobjadr = new long[number_templates.length]; //array of addresses to send to native call


    //load template image(s) for matching
    private void load_templates(){
        try {
            //Load main template
            template_img = Utils.loadResource(this,R.drawable.template_empty);
            template_img.convertTo(template_img, CV_32SC4); //covert to be able to hold negative values

            createTemplateMask(template_img.getNativeObjAddr());
//                normaliseTemplate(template_img.getNativeObjAddr());
//                Imgproc.cvtColor(template_img, template_img, Imgproc.COLOR_RGB2GRAY);
            Log.d(TAG, "load_templates: template type: = " + template_img.type());

            //load number signs
            int[] num_temp_names = {R.drawable.num10, R.drawable.num20, R.drawable.num30,R.drawable.num40,
                    R.drawable.num50,R.drawable.num60,R.drawable.num80};
            for (int i = 0; i < num_temp_names.length; i++) {
                number_templates[i] = Utils.loadResource(this, num_temp_names[i]);
                number_templates[i].convertTo(number_templates[i], CV_32SC4);
                createTemplateMask(number_templates[i].getNativeObjAddr());

//                Imgproc.cvtColor(number_templates[i], number_templates[i], Imgproc.COLOR_RGBA2GRAY);

                tempobjadr[i] =  number_templates[i].getNativeObjAddr();

                Log.d(TAG, "load_templates: number template type: = " + number_templates[i].type());
            }

        } catch (IOException e) {
            e.printStackTrace();
        }
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
            img = Utils.loadResource(this,res);
            Imgproc.cvtColor(img, img, Imgproc.COLOR_RGB2GRAY);
            Log.d(TAG, "load_test_image: loaded successfully");
            img_index++;
        } catch (IOException e) {
            e.printStackTrace();
        }

        return img;
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

                    test_img = load_test_image();

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
        cameraBridgeViewBase = findViewById(R.id.camera_view);
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

        signResponse = test_img.clone();

        //native call
        findSigns(signResponse.getNativeObjAddr(), template_img.getNativeObjAddr(), tempobjadr);

        frameCount++;
        if (frameCount > 20){
            frameCount = 0;
            test_img = load_test_image();
        }

        return signResponse;
    }

    private native void findSigns(long matGrey, long template_img, long[] number_templates);

//    public native void normaliseTemplate(long template_img);

    public native void createTemplateMask(long template_img);
}

