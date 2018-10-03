//
// Created by Gary Rendle on 01/08/2018.
//

#include <jni.h>
#include <string>
#include <sstream>
#include <algorithm>
#include <stack>


#include <android/log.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>


//  __android_log_print(ANDROID_LOG_DEBUG, "display ROIs", "template size: %dx%d, type = %d", number_templates_ref[i].rows, number_templates_ref[i].cols, number_templates_ref[i].type());



class YTemplateMatcher {

public:


    //finds signs, using many other functions below
    static int findSigns(cv::Mat &input, cv::Mat& template_img, std::vector<cv::Mat> *number_template = NULL){

        int speed = -1;

//        std::vector<cv::Rect> good_rects =

        exclude_large_segments(input);

        //downsampling method
        cv::Mat downsampled_img = input;

        const int SCALE_POW = 2;
        downsample(&input, downsampled_img, SCALE_POW);
        std::vector<cv::Rect> rects = get_areas_of_interest(downsampled_img);

//        binarise(input);

        std::vector<int> scores;
//        std::vector<cv::Rect> best_match = matchSigns2(input, rects, template_img, scores);

//        //determine which sign is best described by the best match roi - if templates given
//        if (number_template != NULL && best_match.size() > 0){
//
//            cv::Mat roi(input, best_match[best_match.size()-1]);
//            cv::resize(roi, roi, cv::Size(100,100));
//
//            //TODO find way of recognising number
////            speed = find_speed_on_sign_static_match(roi, number_template);
////            speed = findSpeedOnSign_template_match(roi, number_template);
//
//        }

    displayDownsampledImg(&input, downsampled_img);
    drawRects(input, rects);

//    drawRect(input, best_match);

//        display_rois(input, best_match, template_img, number_template);

//        int rtn_score = -1;
//        if (scores.size() > 0){
//            rtn_score= scores[scores.size()-1];
//        }

        return speed;
    }
//    //finds signs, using many other functions below
//    static int findSigns(cv::Mat &input, cv::Mat& template_img, std::vector<cv::Mat> &number_templates){
//
//
//    }

    //turns template image into a mask, where
    // white = 1, black = -1, transparent = 0
    static void createTemplateMask(cv::Mat &template_img){

        int32_t * template_p;
        //get sum of template
        for (int y = 0; y < template_img.rows; ++y) {
            template_p = template_img.ptr<int32_t>(y);
            for (int x = 0; x < template_img.cols; ++x) {

                //see if there is opacity here
                if (template_p[(x*4)+3] > 0){

                    //set to 1 if white
                    if (template_p[x*template_img.channels()] > 150){
                        template_p[x*template_img.channels()] = 1;
                    }
                        //else set to -1
                    else {
                        template_p[x*template_img.channels()] = -1;
                    }
                }
                    //set to 0 when no opacity is present
                else {
                    template_p[x * template_img.channels()] = 0;
                }
            }
        }
    }

    static void normalise_template(cv::Mat& template_img){

        int32_t * template_p;

        //get sum of template
        long int template_sum = 0;
        long int num_px = 0;
        for (int y = 0; y < template_img.rows; ++y) {
            template_p = template_img.ptr<int32_t>(y);
            for (int x = 0; x < template_img.cols; ++x) {
                //see if there is opacity here
                if (template_p[(x*4)+3] > 0){
                    num_px++;
                    //accumulate intensity
                    template_sum += template_p[x*4];
                }
            }
        }
        long int mean_intensity = template_sum / num_px;
        //subtract mean element wise
        template_img = template_img - cv::Scalar(mean_intensity, mean_intensity, mean_intensity, 0);
    }


private:

    static std::vector<cv::Rect> exclude_large_segments(cv::Mat& input){

        //smothing to reduce effect of noise
        cv::Mat blurred;
        cv::GaussianBlur(input, blurred, cv::Size(7,7), 2);

        cv::Mat lap_result;
        cv::Laplacian(blurred, lap_result, CV_8UC1, 3);
        cv::Mat integral_img;
        cv::integral(lap_result, integral_img);

//        uchar avg_intensity = (uchar)integral_img.ptr<int>(integral_img.rows - 1)[integral_img.cols -1];

//        D/display ROIs: quarter scores 13.29, 12.85, 8.76, 9.40
//        D/display ROIs: quarter scores 13.69, 18.77, 8.75, 10.63

        cv::Rect whole_image (0,0,input.cols, input.rows);

        //subdivide the matrix recursively
        std::vector<cv::Rect> good_rects;
        std::stack<cv::Rect> r_stack;
        r_stack.push(whole_image);

        const float LOW_BOUND = 1.5f;
        const float HIGH_BOUND = 50.f;
        const int MIN_R_WIDTH = 70;

        while (!r_stack.empty()){
            cv::Rect this_rect = r_stack.top();
            r_stack.pop();

            int pxInRect = this_rect.width * this_rect.height;
            int total_gradient = getValueInRect(integral_img, this_rect);
            float avg_grad = (float)total_gradient / pxInRect;


//            __android_log_print(ANDROID_LOG_DEBUG, "display ROIs", "avg_grad = %.2f", avg_grad);

            //if inside bounds....
            if (avg_grad > LOW_BOUND && avg_grad < HIGH_BOUND){


                //check for minimum size
                if (this_rect.width <= MIN_R_WIDTH){

                    //add to list if it is
                    good_rects.push_back(this_rect);
                }
                else {
                    //add children to stack if not
                    int x = this_rect.x; int y = this_rect.y;
                    int h = this_rect.height; int w = this_rect.width;
                    r_stack.push(cv::Rect(x, y , w/2, h/2));
                    r_stack.push(cv::Rect(x + w/2, y , w/2, h/2));
                    r_stack.push(cv::Rect(x, y + h/2 , w/2, h/2));
                    r_stack.push(cv::Rect(x + w/2, y + h/2, w/2, h/2));
                }
            }
            else {
                blackout(input, this_rect);
            }


        }

        return good_rects;
    }

    //switch to 0
    static void blackout(cv::Mat& input, cv::Rect rect){
        uchar* write;
        for (int y = 0; y < rect.height; ++y) {
            write = input.ptr<uchar>(y + rect.y);
            for (int x = 0; x < rect.width; ++x) {
                write[x + rect.x] = 0;
            }
        }
    }

    //integ_img mat type = 4
    static int getValueInRect(cv::Mat &integ_img, cv::Rect rect){

        int* read;

        read = integ_img.ptr<int>(rect.y);
        int a = read[rect.x];
        int b = read[rect.x + rect.width];

        read = integ_img.ptr<int>(rect.y + rect.height);
        int c = read[rect.x];
        int d = read[rect.x + rect.width];

        return (a + d - b - c);
    }


    //iterative downsampling with pyramid function
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

    //increases contrast so that darkest image pixel = 0 and brihghtest = 255
    static void stretch_contrast(cv::Mat& input_image) {

        //find min and max
        double min, max;
        cv::minMaxLoc(input_image, &min, &max);
        input_image = input_image - min;
        input_image = input_image * (255.0 / (max - min));

    }

    //crude threshold based categorisation of pixels as black or white
    static void binarise(cv::Mat& input_image){

        const int THRESHOLD = 128;

        uchar* inpt_ptr;
        for (int y = 0; y < input_image.rows; ++y) {
            inpt_ptr = input_image.ptr<uchar>(y);
            for (int x = 0; x < input_image.cols; ++x) {
                int val = inpt_ptr[x * input_image.channels()];

                if (val > THRESHOLD) {val = 255;}
                else {val = 0;}

                inpt_ptr[x * input_image.channels()] = (uchar)val;
            }
        }
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
    //parametrised size
    static cv::Mat responseFromDownsampledImage(cv::Mat &downsampled_img, int tempSize, int boxSize){

        //create response holder mat at size of downsampled mat
        cv::Mat response = cv::Mat(downsampled_img.rows, downsampled_img.cols, downsampled_img.type());

        const int nRows = downsampled_img.rows - tempSize;
        const int nCols = downsampled_img.cols - tempSize;

        //at each row, build an array of pointers to the required blocks
        uchar* t_ptrs[tempSize];
        uchar* write_p;

        int b_sum = 0, w_sum = 0;

        //for each row
        for (int y = 0; y < nRows; ++y) {

            //build array of read pointers
            for (int p_row = 0; p_row < tempSize; ++p_row) {
                t_ptrs[p_row] = downsampled_img.ptr<uchar>(y + p_row);
            }
            //create write pointer
            write_p = response.ptr<uchar>(y);

            //iterate along the row and perform calculation at each column position to get response
            for (int x = 0; x < nCols; ++x) {

                //collect response from all rows
                int total_diffs = 0;

                //for each row
                for (int t_rows = boxSize; t_rows < (tempSize - boxSize); ++t_rows) {
                    //left side
                    b_sum = 0, w_sum = 0;
                    for (int px = 0; px < boxSize; ++px) { //for each col in row
                        b_sum += t_ptrs[t_rows][x+px];
                        w_sum += t_ptrs[t_rows][x+boxSize+px];
                        total_diffs += (w_sum-b_sum);
                    }
                    //right side
                    b_sum = 0, w_sum = 0;
                    for (int px = 0; px < boxSize; ++px){
                        b_sum += t_ptrs[t_rows][(x+tempSize)-(1+px)];
                        w_sum += t_ptrs[t_rows][(x+tempSize)-(1+px+boxSize)];
                        total_diffs += (w_sum-b_sum);
                    }

                }
                //for each column
                for (int col = boxSize; col < (tempSize-boxSize); ++col) {
                    //top
                    b_sum = 0, w_sum = 0;
                    for (int px = 0; px < boxSize; ++px) {
                        b_sum += t_ptrs[px][x+col];
                        w_sum += t_ptrs[px+boxSize][x+col];
                        total_diffs += (w_sum-b_sum);
                    }
                    //bottom
                    b_sum = 0, w_sum = 0;
                    for (int px = 0; px < boxSize; ++px){
                        b_sum += t_ptrs[tempSize-(1+px)][x+col];
                        w_sum += t_ptrs[tempSize-(1+px+boxSize)][x+col];
                        total_diffs += (w_sum-b_sum);
                    }

                }


                //TODO normalise for number of comparisons?
                total_diffs = total_diffs / ((tempSize-(2*boxSize))*4*boxSize);

                total_diffs = std::max(total_diffs,0);

                write_p[x] = (uchar)(total_diffs/3); // divide to fit into 8bit char
            }
        }

        //transfer response back into downsampled image
//        downsampled_img = response.clone();
        return response.clone();
    }

    //create a likelihood of sign shape from downsampled image
    //parametrised size - 2 parameters
    static cv::Mat responseFromDownsampledImage(cv::Mat &downsampled_img, int tempSize, int boxSize, int cornerSize){

        //create response holder mat at size of downsampled mat
        cv::Mat response = cv::Mat(downsampled_img.rows, downsampled_img.cols, downsampled_img.type());

        const int nRows = downsampled_img.rows - tempSize;
        const int nCols = downsampled_img.cols - tempSize;

        //at each row, build an array of pointers to the required blocks
        uchar* t_ptrs[tempSize];
        uchar* write_p;

        //for each row
        for (int y = 0; y < nRows; ++y) {

            //build array of read pointers
            for (int p_row = 0; p_row < tempSize; ++p_row) {
                t_ptrs[p_row] = downsampled_img.ptr<uchar>(y + p_row);
            }
            //create write pointer
            write_p = response.ptr<uchar>(y);

            //iterate along the row and perform calculation at each column position to get response
            for (int x = 0; x < nCols; ++x) {

                //ignore if corner is black
                if (t_ptrs[0][x] == 0){
                    write_p[x] = 0;
                }
                else {
                    //collect response from all rows
                    int b_sum = 0, w_sum = 0;

                    //for each row
                    for (int t_rows = cornerSize; t_rows < (tempSize - cornerSize); ++t_rows) {
                        //left side
                        for (int px = 0; px < boxSize; ++px) { //for each col in row
                            b_sum += t_ptrs[t_rows][x+px];//edge
                            w_sum += (2 * t_ptrs[t_rows][x+boxSize+px]);//inside edge
                            b_sum += t_ptrs[t_rows][x+(2*boxSize)+px];//centre
                        }
                        //right side
                        for (int px = 0; px < boxSize; ++px){
                            b_sum += t_ptrs[t_rows][(x+tempSize)-(1+px)];//edge
                            w_sum += (2 * t_ptrs[t_rows][(x+tempSize)-(1+px+boxSize)]);//inside edge
                            b_sum += t_ptrs[t_rows][(x+tempSize)-(1+(2*boxSize)+px)];//centre
                        }

                    }
                    //for each column
                    for (int col = cornerSize; col < (tempSize-cornerSize); ++col) {
                        //top
                        for (int px = 0; px < boxSize; ++px) {
                            b_sum += t_ptrs[px][x+col];//edge
                            w_sum += (3 * t_ptrs[px+boxSize][x+col]);//inside edge
                            b_sum += t_ptrs[px+(2*boxSize)][x+col];//centre
                        }
                        //bottom
                        for (int px = 0; px < boxSize; ++px){
                            b_sum += t_ptrs[tempSize-(1+px)][x+col];//edge
                            w_sum += (3 * t_ptrs[tempSize-(1+px+boxSize)][x+col]);//inside edge
                            b_sum += t_ptrs[tempSize-(1+px+(2*boxSize))][x+col];
                        }

                    }

//                //corner subtractions
//                for (int px_x = 1; px_x < cornerSize; ++px_x) {
//                    int px_y = cornerSize - px_x;
//                    //top left
//                    b_sum += t_ptrs[px_y][x+px_x];
//                    //top right
//                    b_sum += t_ptrs[px_y][x+(tempSize-1-px_x)];
//                    //bottom left
//                    b_sum += t_ptrs[tempSize-1-px_y][x+px_x];
//                    //bottm right
//                    b_sum += t_ptrs[tempSize-1-px_y][x+(tempSize-1-px_x)];
//                }

                    int total_diffs = w_sum - b_sum;
                    //normalise for number of comparisons
                    total_diffs = total_diffs / ((tempSize-(2*cornerSize))*4*boxSize);
                    total_diffs = std::max(total_diffs,0);

                    write_p[x] = (uchar)(total_diffs/10); // divide to fit into 8bit char
                }


            }
        }

        //transfer response back into downsampled image
        return response.clone();
    }

    //should get responses from multiple template sizes and find the strongest across sizes
    static std::vector<cv::Rect> get_areas_of_interest(cv::Mat &input_img){

        //define template sizes - move as a global variable?
        std::vector<std::vector<int>> template_sizes;
        template_sizes.push_back(std::vector<int>{6,1,2});
        template_sizes.push_back(std::vector<int>{7,2,2});
        template_sizes.push_back(std::vector<int>{8,2,2});
        template_sizes.push_back(std::vector<int>{9,2,3});
        template_sizes.push_back(std::vector<int>{10,2,3});
        template_sizes.push_back(std::vector<int>{11,2,3});
        template_sizes.push_back(std::vector<int>{12,2,3});
        template_sizes.push_back(std::vector<int>{13,2,3});
        template_sizes.push_back(std::vector<int>{14,2,3});
        template_sizes.push_back(std::vector<int>{15,2,3});
        template_sizes.push_back(std::vector<int>{16,2,4});
        const int NUM_MATS = (int)template_sizes.size();

        //make an array of mats
        const int MIN_SIZE = 4;
        std::vector<cv::Mat> responses;
        for (int i = 0; i < NUM_MATS; ++i) {
            cv::Mat r = cv::Mat(input_img.rows, input_img.cols, input_img.type());
            //get response for each one
            r = responseFromDownsampledImage(input_img, template_sizes[i][0],template_sizes[i][1],template_sizes[i][2]);
            responses.push_back(r);
        }

        // search  mats for rois ===========================

        const int SCALE_POW = 2;
        const int scale = (int)std::pow(2, SCALE_POW); //real px per downsampled px
        const int NUM_RECTS = 40;
        std::vector<cv::Rect> return_rois;
        uchar* write_p;

        //iterate to find most prominent 'peaks'
        for (int i = 0; i < NUM_RECTS; ++i) {

            double global_max = 0.0;
            int max_mat_index=0;
            cv::Point global_max_loc;
            for (int m = 0; m < responses.size(); ++m) {

                cv::Mat search_image = responses[m];

                //find max response value location
                cv::Point min_loc, max_loc;
                double min, max;
                cv::minMaxLoc(search_image, &min, &max, &min_loc, &max_loc);

                if (max > global_max){
                    global_max = max;
                    max_mat_index = m;
                    global_max_loc = max_loc;
                }
            }

            //create rect and add to return list
            cv::Rect ROI (global_max_loc.x * scale, global_max_loc.y * scale, template_sizes[max_mat_index][0] * scale, template_sizes[max_mat_index][0] * scale);
            return_rois.push_back(ROI);

            //block pixels around current from being found again (in all responses)
            //basic local maxima finding system
            const int NBR_RADIUS = 1;
            for (int m = 0; m < responses.size(); ++m) {
                for (int y = -NBR_RADIUS; y <= NBR_RADIUS; ++y) {
                    write_p = responses[m].ptr<uchar>(global_max_loc.y + y);
                    for (int x = -NBR_RADIUS; x <= NBR_RADIUS; ++x) {
                        write_p[global_max_loc.x + x] = (uchar)(0);
                    }
                }
            }



        }

        return return_rois;
    }

//    static std::vector<cv::Rect> get_areas_of_interest(cv::Mat &downsampled_img, int scale_iterations, int template_size){
//
//        const int scale = (int)std::pow(2, scale_iterations);
//        const int NUM_RECTS = 5;
//        std::vector<cv::Rect> return_rois;
//        uchar* write_p;
//
//        //create a new matrix to hold edited data
//        cv::Mat search_image = downsampled_img.clone();
//
//        //iterate to find most prominent 'peaks'
//        for (int i = 0; i < NUM_RECTS; ++i) {
//
//            //find max response value location
//            cv::Point min_loc, max_loc;
//            double min, max;
//            cv::minMaxLoc(search_image, &min, &max, &min_loc, &max_loc);
//
//            //create rect and add to return list
//            cv::Rect ROI (max_loc.x * scale, max_loc.y * scale, template_size * scale, template_size * scale);
//            return_rois.push_back(ROI);
//
//            //set value at that point to 0 (so it is not found again)
//            write_p = search_image.ptr<uchar>(max_loc.y);
//            write_p[max_loc.x] = (uchar)(0);
//
//        }
//
//        return return_rois;
//
//    }

    static std::vector<cv::Rect> matchSigns1(cv::Mat input_img, std::vector<cv::Rect> rois, cv::Mat &template_img){

        //to keep scores in:
        std::vector<long int> roi_scores(rois.size());

        //for each rect
        for (int i = 0; i < rois.size(); ++i) {

            //scale roi to same size as templates (100x100)
            cv::Mat roi = cv::Mat(input_img, rois[i]);
            cv::resize(roi, roi, cv::Size(template_img.rows, template_img.cols));

            long int response = match_template_sqdiff(roi, template_img);
            roi_scores[i] = response;

        }

        //rearrange rois by order (min to max)
        std::vector<cv::Rect> rtn_rects (rois.size());
        for (int i = 0; i < rois.size(); ++i) {
            std::vector<long int>::iterator min = std::min_element(std::begin(roi_scores), std::end(roi_scores));
            long int min_index = std::distance(std::begin(roi_scores), min);
            roi_scores[min_index] = LONG_MAX;
            rtn_rects[i] = rois[min_index];
        }

        //return best matches
        return rtn_rects;
    }

    //self made template matching for sqdiff TM
    //requires that mats are pre-sized correctly
    static long int match_template_sqdiff(cv::Mat &input_img, cv::Mat &template_img){

        //check dimensions of input mats
        CV_Assert(input_img.rows == template_img.rows && input_img.cols == template_img.cols);

        long int response = 0;

        int nRows = input_img.rows;
        int nCols = input_img.cols;

        int32_t * template_p;
        uchar* img_p;

        //for each pixel
        for (int y = 0; y < nRows; ++y) {
            img_p = input_img.ptr<uchar>(y);
            template_p = template_img.ptr<int32_t>(y);

            for (int x = 0; x < nCols; ++x) {

                //check alpha value of template
                //skip comparison if 0
                int32_t template_alpha = template_p[(x*4)+3];

                if (template_alpha > 0){
                    int32_t template_val = template_p[x*4];
                    int32_t img_val = img_p[x];

                    //add squared difference to response
                    response = (template_val - img_val) * (template_val - img_val);
                }

            }
        }
        return response;

    }

    static std::vector<cv::Rect> matchSigns2(cv::Mat input_img, std::vector<cv::Rect> rois, cv::Mat &template_img, std::vector<int> &scores){

        //to keep scores in:
        std::vector<long int> roi_scores(rois.size());

        //for each rect
        for (int i = 0; i < rois.size(); ++i) {

            //scale roi to same size as templates (100x100)
            cv::Mat roi = cv::Mat(input_img, rois[i]);
            cv::resize(roi, roi, cv::Size(template_img.rows, template_img.cols));

            long int response = match_template_mask(roi, template_img);
            roi_scores[i] = response;

        }

        //rearrange rois by order (min to max)
//        std::vector<cv::Rect> rtn_rects (rois.size());
        std::vector<cv::Rect> rtn_rects;


        for (int i = 0; i < rois.size(); ++i) {
            std::vector<long int>::iterator min = std::min_element(std::begin(roi_scores), std::end(roi_scores));
            long int min_index = std::distance(std::begin(roi_scores), min);



//            scores.push_back((int)roi_scores[min_index]);

//            if (roi_scores[min_index] > 0){
                rtn_rects.push_back(rois[min_index]);
//            }

            roi_scores[min_index] = LONG_MAX;
//            rtn_rects[i] = rois[min_index];
        }

        //return best matches
        return rtn_rects;
    }

    //requires that mats are pre-sized correctly
    //treats template as a mask, expects -1 for black values and 1 for white values
    static long int match_template_mask(cv::Mat &input_img, cv::Mat &template_img){

        //check dimensions of input mats
        CV_Assert(input_img.rows == template_img.rows && input_img.cols == template_img.cols);

        long int response = 0;

        int nRows = input_img.rows;
        int nCols = input_img.cols;

        int32_t * template_p;
        uchar* img_p;

        //for each pixel
        for (int y = 0; y < nRows; ++y) {
            img_p = input_img.ptr<uchar>(y);
            template_p = template_img.ptr<int32_t>(y);

            for (int x = 0; x < nCols; ++x) {

                int32_t template_val = template_p[x*template_img.channels()];
                int32_t img_val = img_p[x];
                response += (template_val * img_val);
            }
        }
        return response;
    }



    //draws a list of rects on a given image
    static void drawRects(cv::Mat& input, std::vector<cv::Rect> rects){
        for (int i = 0; i < rects.size(); ++i) {
            cv::rectangle(input,rects[i], cv::Scalar(255));
        }
    }
    //draws a list of rects on a given image
    static void drawRect(cv::Mat& input, cv::Rect rect){
            cv::rectangle(input,rect, cv::Scalar(255));
    }

    //prints all rois to screen (not checking length of rects vector atm)
    static void display_rois(cv::Mat &input,
                             std::vector<cv::Rect> rects,
                             cv::Mat& template_img,
                             std::vector<cv::Mat> *number_template = NULL){

        cv::Mat output (input.rows, input.cols, input.type());
        const int ROIS_IN_ROW = 10;
        uchar* write;
        uchar* read;

        //write rois to output image
        for (int i = 0; i < rects.size(); ++i) {

            int roi_size = 100;

            cv::Mat roi = cv::Mat(input, rects[i]);
            cv::resize(roi, roi, cv::Size(roi_size,roi_size));

            int col = i % ROIS_IN_ROW;
            int row = i / ROIS_IN_ROW;
            int start_x = col * 120;
            int start_y = row * 120;

            //write to output


            //write roi to output
            for (int y = 0; y < roi_size; ++y) {

                read = roi.ptr<uchar>(y);
                write = output.ptr<uchar>(start_y + y);
                for (int x = 0; x < roi_size; ++x) {
                    write[start_x + x] = read[x];
                }
            }
        }


        //write template to output
//        int32_t * read;
//
//        int tmp_start_x = 0;
//
//        for (int y = 0; y < template_img.rows; ++y) {
//            read = template_img.ptr<int32_t>(y);
//
//            write = output.ptr<uchar>(tmp_start_y + y);
//            for (int x = 0; x < template_img.cols; ++x) {
//                int32_t offset = 0;
//                int32_t write_val = read[x * template_img.channels()] + offset;
//                write_val = std::min(write_val, 255);
//                write_val = std::max(write_val, 0);
//                write[tmp_start_x + x] = (uchar)write_val;
//            }
//        }


        int tmp_start_y = 800;

//        //display number templates - from image files
//        std::vector<cv::Mat>& number_templates_ref = *number_template;
//        if (number_template != NULL){
//            int offset = 120;
//            for (int i = 0; i < number_templates_ref.size(); ++i) {
//                //output type and size
//
//                for (int y = 0; y < number_templates_ref[i].rows; ++y) {
//                    read = number_templates_ref[i].ptr<uchar>(y);
//                    write = output.ptr<uchar>(tmp_start_y + y);
//                    for (int x = 0; x < number_templates_ref[i].cols; ++x) {
//                        write[x + (i*offset)] = read[x * number_templates_ref[i].channels()];
//                    }
//                }
//            }
//        }

        //display number masks
        std::vector<cv::Mat>& number_templates_ref = *number_template;
        if (number_template != NULL){
            int offset = 120;
            int32_t *read_i;
            for (int i = 0; i < number_templates_ref.size(); ++i) {
                for (int y = 0; y < number_templates_ref[i].rows; ++y) {
                    read_i = number_templates_ref[i].ptr<int32_t>(y);
                    write = output.ptr<uchar>(tmp_start_y + y);
                    for (int x = 0; x < number_templates_ref[i].cols; ++x) {
                        int32_t color = read_i[x * number_templates_ref[i].channels()];
                        if (color == 1){
                            color = 255;
                        }
                        else {
                            color = 0;
                        }
                        write[x + (i*offset)] = (uchar)color;
                    }
                }
            }
        }

        input = output.clone();
    }

    //template type = 28, input type = 0
    static int find_speed_on_sign_static_match(cv::Mat &input, std::vector<cv::Mat> *number_templates){
//        int speed = 0;

        std::vector<cv::Mat>& number_templates_ref = *number_templates;

        //check dimensions of input mat/template
        CV_Assert(input.rows == number_templates_ref[0].rows && input.cols == number_templates_ref[0].cols);

        //simple mask system
        uchar* read_input;
        int32_t* read_templ;
        int bestMatch = 0;
        int bestMatchId = 0;
        for (int i = 0; i < number_templates_ref.size(); ++i) {
            int response = 0;
            for (int y = 0; y < input.rows; ++y) {
                read_input = input.ptr<uchar>(y);
                read_templ = number_templates_ref[i].ptr<int32_t>(y);
                for (int x = 0; x < input.cols; ++x) {
                    response += read_input[x] * read_templ[x];
                }
            }
            if (response > bestMatch){
                bestMatch = response;
                bestMatchId = i;
            }
              __android_log_print(ANDROID_LOG_DEBUG, "find speed", "template id: %d, score = %d", i,response);
        }

        return bestMatchId;
    }

    //uses a template matching approach to move each template across input
    //to find number with best match
    static int findSpeedOnSign_template_match(cv::Mat &input, std::vector<cv::Mat> *number_templates){
        int speed = 0;

        std::vector<cv::Mat>& number_templates_ref = *number_templates;
        std::vector<int> num_scores (number_templates_ref.size());

        int bestMatch = 0, bestMatchId = 0;

        for (int i = 0; i < number_templates_ref.size(); ++i) {
            num_scores[i] = match_number_template_maskwise(input, number_templates_ref[i]);

            if (num_scores[i] > bestMatch){
                bestMatchId = i;
            }

            __android_log_print(ANDROID_LOG_DEBUG, "find speed tm", "template id: %d, score = %d", i,num_scores[i]);
        }
        return num_scores[bestMatchId];
    }

    //number template matching
    //template type = 28, input type = 0
    static int match_number_template_maskwise(cv::Mat &input, cv::Mat &num_template){

        cv::Mat result(input.rows - num_template.rows + 1,
                        input.cols - num_template.cols + 1,
                            CV_32SC1);

//        __android_log_print(ANDROID_LOG_DEBUG, "match_number_template_maskwise",
//                            "input size = %dx%d, template size = %dx%d, result size = %dx%d, ",
//                            input.rows, input.cols, num_template.rows, num_template.cols, result.rows, result.cols  );


        //match template at each possible step and record results in result mat
        for (int y = 0; y < result.rows; ++y) {
            for (int x = 0; x < result.cols; ++x) {

                int32_t* result_write = result.ptr<int32_t>(y);
                int32_t response = 0;

                //calculate response
                for (int y_read = 0; y_read < num_template.rows; ++y_read) {

                    uchar* read_input = input.ptr<uchar>(y + y_read);
                    int32_t* read_template = num_template.ptr<int32_t>(y + y_read);

                    for (int x_read = 0; x_read < num_template.cols; ++x_read) {

                         response += (int)read_input[x+x_read] * (int)read_template[(x+x_read)*num_template.channels()];
                    }
                }
                result_write[x] = response;
            }
        }

        double min, max;
        cv::minMaxLoc(result, &min, &max);

        return (int) max;
    }

};

//sign finder calls -------------------------------

extern "C" JNIEXPORT void
JNICALL
Java_com_example_garyrendle_mis_1cpp_1test_SignFinderPhotoTest_findSigns(
        JNIEnv* env, jobject,
        jlong input_ptr,
        jlong template_ptr,
        jlongArray number_templates_arr){

    cv::Mat& input = *(cv::Mat *) input_ptr;
    cv::Mat& template_img = *(cv::Mat *)template_ptr;


    YTemplateMatcher::findSigns(input, template_img);

    //ref: stackoverflow.com/questions/20193039/how-to-pass-a-arraylistmat-from-java-to-native-sidendk-in-opencv-for-android
//    std::vector<cv::Mat> number_templates;
//    jsize a_len = env->GetArrayLength(number_templates_arr);
//    jlong *traindata = env->GetLongArrayElements(number_templates_arr,0);
//
//    for(int k=0;k<a_len;k++)
//    {
//        cv::Mat & newimage=*(cv::Mat*)traindata[k];
//        number_templates.push_back(newimage);
//
//    }
//    YTemplateMatcher::findSigns(input, template_img, &number_templates);
//    env->ReleaseLongArrayElements(number_templates_arr,traindata,0);

}

//TODO sort repeat
extern "C" JNIEXPORT void
JNICALL
Java_com_example_garyrendle_mis_1cpp_1test_SignFinderCamTest_findSigns(
        JNIEnv* env, jobject,
        jlong input_ptr,
        jlong template_ptr){

    cv::Mat& input = *(cv::Mat *) input_ptr;
    cv::Mat& template_img = *(cv::Mat *)template_ptr;
    YTemplateMatcher::findSigns(input, template_img);
}


//TODO sort repeat
extern "C" JNIEXPORT jint
JNICALL
Java_com_example_garyrendle_mis_1cpp_1test_SignFinderBackground_findSigns(
        JNIEnv* env, jobject,
        jlong gray,
        jlong template_ptr){

    cv::Mat& input = *(cv::Mat *) gray;
    cv::Mat& template_img = *(cv::Mat *)template_ptr;

    return (jint)YTemplateMatcher::findSigns(input, template_img);
}


//function to normalise templates so that sum of intensity = 0------------------------
extern "C" JNIEXPORT void
JNICALL
Java_com_example_garyrendle_mis_1cpp_1test_SignFinderPhotoTest_normaliseTemplate(
        JNIEnv* env, jobject,
        jlong template_ptr){

    cv::Mat& template_img = *(cv::Mat *)template_ptr;
    YTemplateMatcher::normalise_template(template_img);
}

//to create template mask------------------------

extern "C" JNIEXPORT void
JNICALL
Java_com_example_garyrendle_mis_1cpp_1test_SignFinderPhotoTest_createTemplateMask(
        JNIEnv* env, jobject,
        jlong template_ptr){

    cv::Mat& template_img = *(cv::Mat *)template_ptr;
    YTemplateMatcher::createTemplateMask(template_img);
}

extern "C" JNIEXPORT void
JNICALL
Java_com_example_garyrendle_mis_1cpp_1test_SignFinderCamTest_createTemplateMask(
        JNIEnv* env, jobject,
        jlong template_ptr){

    cv::Mat& template_img = *(cv::Mat *)template_ptr;
    YTemplateMatcher::createTemplateMask(template_img);
}

extern "C" JNIEXPORT void
JNICALL
Java_com_example_garyrendle_mis_1cpp_1test_SignFinderBackground_createTemplateMask(
        JNIEnv* env, jobject,
        jlong template_ptr){

    cv::Mat& template_img = *(cv::Mat *)template_ptr;
    YTemplateMatcher::createTemplateMask(template_img);
}











