//
// Created by Gary Rendle on 01/08/2018.
//

#include <jni.h>
#include <string>
#include <sstream>
#include <algorithm>

#include <android/log.h>


#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>


class YTemplateMatcher {

public:


    //iterative downsampling with pyramid function
    //TODO - can this be done directly without downsampling iteratively?
    static void downsample(cv::Mat* input, cv::Mat& output, int iterations){
        cv::Mat temp, downsampled_img;
        temp = *input;
        downsampled_img = temp;
        for (int i = 0; i < iterations; ++i) {
            cv::pyrDown( temp, downsampled_img, cv::Size( temp.cols/2, temp.rows/2 ) );
            temp = downsampled_img;
        }
        output = downsampled_img;
    }

    //transfer downsampled data back to full size mat
    static void displayDownsampledImg(cv::Mat* responseDest,
                                 cv::Mat &downsampled_img){
        int nRows = responseDest->rows;
        int nCols = responseDest->cols;
        int scale = responseDest->rows / downsampled_img.rows;
        uchar* p;
        uchar* ds_p; //pointer to downsampled image
        for( int y = 0; y < nRows; ++y)
        {
            //get ptr to this row in return mat
            p = responseDest->ptr<uchar>(y);
            ds_p = downsampled_img.ptr<uchar>(y/scale);

            for (int x = 0; x < nCols; ++x)
            {
                p[x] = ds_p[x/scale];
            }
        }
    }

    //create a likelihood of sign shape from downsampled image
    static void responseFromDownsampledImage(cv::Mat &downsampled_img){

        // accept only 8 bit single channel input
        CV_Assert(downsampled_img.type() == 0);

        //create response holder mat at size of downsampled mat
        cv::Mat response = cv::Mat(downsampled_img.rows, downsampled_img.cols, downsampled_img.type());

        const int templateSize = 4; //width and height of template in downsampled pixels (blocks). assume square.
        const int nRows = downsampled_img.rows - templateSize;
        const int nCols = downsampled_img.cols - templateSize;

        //at each row, build an array of pointers to the required blocks
        uchar* t_ptrs[templateSize];
        uchar* write_p;
        int max_response = 0;
        for (int y = 0; y < nRows; ++y) {

            //build array of read pointers
            for (int p_row = 0; p_row < templateSize; ++p_row) {
                t_ptrs[p_row] = downsampled_img.ptr<uchar>(y + p_row);
            }
            //create write pointer
            write_p = response.ptr<uchar>(y);

            //iterate along the row and perform calculation at each column position to get response
            for (int x = 0; x < nCols; ++x) {

                //collect response from all rows
                int total_diffs = 0;

                //pairwise comparisons
//                total_diffs += abs(t_ptrs[1][x+1] - t_ptrs[0][x+1]); //1
//                total_diffs += abs(t_ptrs[1][x+2] - t_ptrs[0][x+2]); //2
//                total_diffs += abs(t_ptrs[1][x+2] - t_ptrs[1][x+3]); //3
//                total_diffs += abs(t_ptrs[2][x+2] - t_ptrs[2][x+3]); //4
//                total_diffs += abs(t_ptrs[2][x+2] - t_ptrs[3][x+2]); //5
//                total_diffs += abs(t_ptrs[2][x+1] - t_ptrs[3][x+1]); //6
//                total_diffs += abs(t_ptrs[2][x+1] - t_ptrs[2][x]); //7
//                total_diffs += abs(t_ptrs[1][x+1] - t_ptrs[1][x]); //8


                total_diffs += (t_ptrs[1][x+1] - t_ptrs[0][x+1]); //1
                total_diffs += (t_ptrs[1][x+2] - t_ptrs[0][x+2]); //2
                total_diffs += (t_ptrs[1][x+2] - t_ptrs[1][x+3]); //3
                total_diffs += (t_ptrs[2][x+2] - t_ptrs[2][x+3]); //4
                total_diffs += (t_ptrs[2][x+2] - t_ptrs[3][x+2]); //5
                total_diffs += (t_ptrs[2][x+1] - t_ptrs[3][x+1]); //6
                total_diffs += (t_ptrs[2][x+1] - t_ptrs[2][x]); //7
                total_diffs += (t_ptrs[1][x+1] - t_ptrs[1][x]); //8


//                total_diffs += std::max(t_ptrs[1][x+1] - t_ptrs[0][x+1], 0); //1
//                total_diffs += std::max(t_ptrs[1][x+2] - t_ptrs[0][x+2], 0); //2
//                total_diffs += std::max(t_ptrs[1][x+2] - t_ptrs[1][x+3], 0); //3
//                total_diffs += std::max(t_ptrs[2][x+2] - t_ptrs[2][x+3], 0); //4
//                total_diffs += std::max(t_ptrs[2][x+2] - t_ptrs[3][x+2], 0); //5
//                total_diffs += std::max(t_ptrs[2][x+1] - t_ptrs[3][x+1], 0); //6
//                total_diffs += std::max(t_ptrs[2][x+1] - t_ptrs[2][x], 0); //7
//                total_diffs += std::max(t_ptrs[1][x+1] - t_ptrs[1][x], 0); //8


                total_diffs = std::max(total_diffs,0);

                write_p[x] = (uchar)(total_diffs/3); // divide to fit into 8bit char

                if (total_diffs > max_response){max_response = total_diffs;}

            }

//            __android_log_print(ANDROID_LOG_ERROR, "SignFinder", "max response = %d", max_response);



        }

        //transfer response back into downsampled image
        downsampled_img = response.clone();

    }

    static std::vector<cv::Rect> get_areas_of_interest(cv::Mat &downsampled_img, int scale_iterations, int template_size){

        const int scale = (int)std::pow(2, scale_iterations);
        const int NUM_RECTS = 5;
        std::vector<cv::Rect> return_rois;
        uchar* write_p;

        //create a new matrix to hold edited data
        cv::Mat search_image = downsampled_img.clone();


        //iterate to find most prominent 'peaks'
        for (int i = 0; i < NUM_RECTS; ++i) {

            //find max response value location
            cv::Point min_loc, max_loc;
            double min, max;
            cv::minMaxLoc(search_image, &min, &max, &min_loc, &max_loc);

            //create rect and add to return list
            cv::Rect ROI (max_loc.x * scale, max_loc.y * scale, template_size * scale, template_size * scale);
            return_rois.push_back(ROI);

            //set value at that point to 0 (so it is not found again)
            write_p = search_image.ptr<uchar>(max_loc.y);
            write_p[max_loc.x] = (uchar)(0);


        }

        return return_rois;

    }

    //draws a list of rects on a given image
    static void drawRects(cv::Mat& input, std::vector<cv::Rect> rects){
        for (int i = 0; i < rects.size(); ++i) {
            cv::rectangle(input,rects[i], cv::Scalar(255));
        }
    }

    //fills the response matrix
    static void rectanglePatternMatching_integralimg(cv::Mat* responseDest,
                                                     cv::Mat* input_integ_img) {

        // accept only single channel input
        CV_Assert(responseDest->channels() == 1);

        //for each position in matrix calculate response
        //TODO - border handling? for now only calc when template is in image range

        int max_response = 0;
        //access each element of return array - within range
        int nRows = responseDest->rows - templateSize;
        int nCols = responseDest->cols - templateSize;
        uchar* p;
        for( int y = 0; y < nRows; ++y)
        {
            //get ptr to this row
            p = responseDest->ptr<uchar>(y);
            for (int x = 0; x < nCols; ++x)
            {
                //calculate response at each part - cast to 8 bit
                int response = getResponseAt(x,y,input_integ_img);
                if (response > max_response) {max_response = response;}
                p[x] = (uchar)(response / 1000);//hard coded scaling - only for visualisation purposes
            }
        }

    }

private:
    static const int templateSize = 50;

    //calculates the total differences between pairs of regions
    //where top let of template is given by x,y
     static int getResponseAt(int x, int y, cv::Mat* input_integ_img) {

        int totalDiffs = 0, bSum = 0, wSum = 0;


        //hard coding of template - means no template has to be passed in to JNI call
//        wSum = getSumInRect(new cv::Rect(x,y, templateSize, templateSize),  input_integ_img);
//        bSum = getSumInRect(new cv::Rect(x,y, templateSize, templateSize),  input_integ_img);

//        //1
//        wSum = getSumInRect(new cv::Rect(x + (templateSize / 4), y + (templateSize / 8), templateSize/2, templateSize/8),  input_integ_img);
//        bSum = getSumInRect(new cv::Rect(x + (templateSize / 4), y, templateSize/2, templateSize/8),  input_integ_img);
//        totalDiffs += abs(wSum - bSum);
//
//        //2
//        wSum = getSumInRect(new cv::Rect(x + (templateSize * 0.75),y + (templateSize / 4), templateSize / 8, templateSize / 2),  input_integ_img);
//        bSum = getSumInRect(new cv::Rect(x + (templateSize * 0.875),y + (templateSize / 4), templateSize / 8, templateSize / 2), input_integ_img);
//        totalDiffs += abs(wSum - bSum);
//
//        //3
//        wSum = getSumInRect(new cv::Rect(x + (templateSize / 4),y + (templateSize * 0.75), templateSize / 2, templateSize / 8),  input_integ_img);
//        bSum = getSumInRect(new cv::Rect(x + (templateSize / 4),y + (templateSize * 0.875), templateSize / 2, templateSize / 8),  input_integ_img);
//        totalDiffs += abs(wSum - bSum);
//
//        //4
//        wSum = getSumInRect(new cv::Rect(x +(templateSize/8),y + (templateSize / 4), templateSize / 8, templateSize / 2),  input_integ_img);
//        bSum = getSumInRect(new cv::Rect(x,y + (templateSize / 4), templateSize / 8, templateSize / 2),  input_integ_img);
//        totalDiffs += abs(wSum - bSum);

//        //5
//        wSum = getSumInRect(new cv::Rect(x,y, templateSize, templateSize),  input_integ_img);
//        bSum = getSumInRect(new cv::Rect(x,y, templateSize, templateSize),  input_integ_img);
//        totalDiffs += abs(wSum - bSum);
//
//        //6
//        wSum = getSumInRect(new cv::Rect(x,y, templateSize, templateSize),  input_integ_img);
//        bSum = getSumInRect(new cv::Rect(x,y, templateSize, templateSize),  input_integ_img);
//        totalDiffs += abs(wSum - bSum);
//
//        //7
//        wSum = getSumInRect(new cv::Rect(x,y, templateSize, templateSize),  input_integ_img);
//        bSum = getSumInRect(new cv::Rect(x,y, templateSize, templateSize),  input_integ_img);
//        totalDiffs += abs(wSum - bSum);
//
//        //8
//        wSum = getSumInRect(new cv::Rect(x,y, templateSize, templateSize),  input_integ_img);
//        bSum = getSumInRect(new cv::Rect(x,y, templateSize, templateSize),  input_integ_img);
//        totalDiffs += abs(wSum - bSum);

        //simplified single pair: W - B
//        W
        totalDiffs += getSumInRect(new cv::Rect(x + (templateSize/2),y,templateSize/2, templateSize),  input_integ_img);
        //B
        totalDiffs -= getSumInRect(new cv::Rect(x,y,templateSize/2, templateSize),  input_integ_img);
        totalDiffs = abs(totalDiffs);

//        __android_log_print(ANDROID_LOG_ERROR, "SEARCH FOR THIS TAG", "totalDiffs + %d", abs(totalDiffs));

        return totalDiffs;
    }

    //returns sum of pixel intensities within the given rectangle,
    // using an integral image which it takes as an argument
    static int getSumInRect(cv::Rect *r, cv::Mat* input_integ_img){

        //use int as type because integ img is type 4: 32 bit int
        int a = input_integ_img->at<int>(r->y, r->x);
        int b = input_integ_img->at<int>(r->y, r->x+r->width);
        int c = input_integ_img->at<int>(r->y + r->height, r->x);
        int d = input_integ_img->at<int>(r->y + r->height, r->x+r->width);
        return (a + d - b - c);
    }



};


extern "C" JNIEXPORT void
JNICALL
Java_com_example_garyrendle_mis_1cpp_1test_SignFinder_findSigns(
        JNIEnv* env, jobject,
        jlong gray,
        jlong integ_img_placeholder){

    cv::Mat& input = *(cv::Mat *) gray;

    //TODO address memory leak - def occurs when using integ_img method

    //method that first creates integral image to calculate region totals:
//    cv::Mat& integ_img = *(cv::Mat *) integ_img_placeholder;
//    cv::integral(input, integ_img);
//    YTemplateMatcher::rectanglePatternMatching_integralimg(&input, &integ_img);


    //downsampling method
    cv::Mat downsampled_img = input;
    YTemplateMatcher::downsample(&input, downsampled_img,4);
    YTemplateMatcher::responseFromDownsampledImage(downsampled_img);
    std::vector<cv::Rect> rects = YTemplateMatcher::get_areas_of_interest(downsampled_img, 4, 4);
    YTemplateMatcher::displayDownsampledImg(&input, downsampled_img);
    YTemplateMatcher::drawRects(input, rects);

    //TODO how best to create response from downsampled image?

}






