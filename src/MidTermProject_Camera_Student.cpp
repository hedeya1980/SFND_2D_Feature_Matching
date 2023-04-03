/* INCLUDES FOR THIS PROJECT */
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <limits>
#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/xfeatures2d/nonfree.hpp>

#include "dataStructures.h"
#include "matching2D.hpp"

using namespace std;

/* MAIN PROGRAM */
int main(int argc, const char *argv[])
{

    /* INIT VARIABLES AND DATA STRUCTURES */

    // data location
    string dataPath = "../";

    // camera
    string imgBasePath = dataPath + "images/";
    string imgPrefix = "KITTI/2011_09_26/image_00/data/000000"; // left camera, color
    string imgFileType = ".png";
    int imgStartIndex = 0; // first file index to load (assumes Lidar and camera names have identical naming convention)
    int imgEndIndex = 9;   // last file index to load
    int imgFillWidth = 4;  // no. of digits which make up the file index (e.g. img-0001.png)

    // misc
    int dataBufferSize = 2;       // no. of images which are held in memory (ring buffer) at the same time
    vector<DataFrame> dataBuffer; // list of data frames which are held in memory at the same time
    bool bVis = false;            // visualize results

    /* MAIN LOOP OVER ALL IMAGES */
    vector<int> Total_KP_list;
    vector<int> KP_list;
    vector<float> detectionTime;
    vector<float> descExtractionTime;
    vector<int> KP_matches_list;



    for (size_t imgIndex = 0; imgIndex <= imgEndIndex - imgStartIndex; imgIndex++)
    {
        /* LOAD IMAGE INTO BUFFER */
        // assemble filenames for current index
        ostringstream imgNumber;
        imgNumber << setfill('0') << setw(imgFillWidth) << imgStartIndex + imgIndex;
        string imgFullFilename = imgBasePath + imgPrefix + imgNumber.str() + imgFileType;

        // load image from file and convert to grayscale
        cv::Mat img, imgGray;
        img = cv::imread(imgFullFilename);
        cv::cvtColor(img, imgGray, cv::COLOR_BGR2GRAY);

        //// STUDENT ASSIGNMENT
        //// TASK MP.1 -> replace the following code with ring buffer of size dataBufferSize

        // push image into data frame buffer
        DataFrame frame;
        frame.cameraImg = imgGray;
        if(dataBuffer.size()<dataBufferSize)
            dataBuffer.push_back(frame);
        else
        {
            dataBuffer.erase(dataBuffer.begin());
            dataBuffer.push_back(frame);
        }
        //cout<<"Buffer size = "<<dataBuffer.size()<<"\n";
        //// EOF STUDENT ASSIGNMENT
        cout << "#1 : LOAD IMAGE INTO BUFFER done" << endl;

        /* DETECT IMAGE KEYPOINTS */

        // extract 2D keypoints from current image
        double t_detection = (double)cv::getTickCount();
        vector<cv::KeyPoint> keypoints; // create empty feature list for current image
        //string detectorType = "SHITOMASI";
        //string detectorType = "HARRIS";
        string detectorType = "ORB";

        //// STUDENT ASSIGNMENT
        //// TASK MP.2 -> add the following keypoint detectors in file matching2D.cpp and enable string-based selection based on detectorType
        //// -> HARRIS, FAST, BRISK, ORB, AKAZE, SIFT

        if (detectorType.compare("SHITOMASI") == 0)
        {
            detKeypointsShiTomasi(keypoints, imgGray, false);
        }
        else if (detectorType.compare("HARRIS") == 0)
        {
            detKeypointsHarris(keypoints, imgGray, false);
        }        
        else
        {
            detKeypointsModern(keypoints, imgGray, detectorType, false);
        }
        t_detection = ((double)cv::getTickCount() - t_detection) / cv::getTickFrequency();
        detectionTime.push_back(1000*t_detection/1);
        //// EOF STUDENT ASSIGNMENT

        //// STUDENT ASSIGNMENT
        //// TASK MP.3 -> only keep keypoints on the preceding vehicle
        Total_KP_list.push_back(keypoints.size());
        // only keep keypoints on the preceding vehicle
        bool bFocusOnVehicle = true;
        cv::Rect vehicleRect(535, 180, 180, 150);
        if (bFocusOnVehicle)
        {
            // ...
            vector <cv::KeyPoint> rectKeypoints;
            for (auto k:keypoints)
            {
                if (vehicleRect.contains(k.pt))
                    rectKeypoints.push_back(k);
            }
            keypoints=rectKeypoints;
            cv::Mat visImage = img.clone();
            cv::drawKeypoints(img, keypoints, visImage, cv::Scalar::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
            string windowName = "Rect Keypoints\n";
            cv::namedWindow(windowName, 2);
            imshow(windowName, visImage);
            cout<<"Rect keypoints = "<<keypoints.size()<< endl;
            cv::waitKey(0);

        }

        //// EOF STUDENT ASSIGNMENT

        // optional : limit number of keypoints (helpful for debugging and learning)
        bool bLimitKpts = false;
        if (bLimitKpts)
        {
            int maxKeypoints = 50;

            if (detectorType.compare("SHITOMASI") == 0)
            { // there is no response info, so keep the first 50 as they are sorted in descending quality order
                keypoints.erase(keypoints.begin() + maxKeypoints, keypoints.end());
            }
            cv::KeyPointsFilter::retainBest(keypoints, maxKeypoints);
            cout << " NOTE: Keypoints have been limited!" << endl;
        }

        // push keypoints and descriptor for current frame to end of data buffer
        (dataBuffer.end() - 1)->keypoints = keypoints;

        cout << "#2 : DETECT KEYPOINTS done" << endl;

        /* EXTRACT KEYPOINT DESCRIPTORS */

        //// STUDENT ASSIGNMENT
        //// TASK MP.4 -> add the following descriptors in file matching2D.cpp and enable string-based selection based on descriptorType
        //// -> BRIEF, ORB, FREAK, AKAZE, SIFT
        double t_desc_extraction = (double)cv::getTickCount();
    
        cv::Mat descriptors;
        string descriptorType = "BRIEF"; // BRIEF, ORB, FREAK, AKAZE, SIFT
        descKeypoints((dataBuffer.end() - 1)->keypoints, (dataBuffer.end() - 1)->cameraImg, descriptors, descriptorType);
        //// EOF STUDENT ASSIGNMENT
        t_desc_extraction = ((double)cv::getTickCount() - t_desc_extraction) / cv::getTickFrequency();
        descExtractionTime.push_back(1000*t_desc_extraction/1);

        // push descriptors for current frame to end of data buffer
        (dataBuffer.end() - 1)->descriptors = descriptors;

        cout << "#3 : EXTRACT DESCRIPTORS done" <<", descriptors = "<<descriptors.size()<< endl;

        if (dataBuffer.size() > 1) // wait until at least two images have been processed
        {

            /* MATCH KEYPOINT DESCRIPTORS */

            vector<cv::DMatch> matches;
            string matcherType = "MAT_BF";        // MAT_BF, MAT_FLANN
            string descriptorType = "DES_BINARY"; // DES_BINARY, DES_HOG
            string selectorType = "SEL_KNN";       // SEL_NN, SEL_KNN

            //// STUDENT ASSIGNMENT
            //// TASK MP.5 -> add FLANN matching in file matching2D.cpp
            //// TASK MP.6 -> add KNN match selection and perform descriptor distance ratio filtering with t=0.8 in file matching2D.cpp

            matchDescriptors((dataBuffer.end() - 2)->keypoints, (dataBuffer.end() - 1)->keypoints,
                             (dataBuffer.end() - 2)->descriptors, (dataBuffer.end() - 1)->descriptors,
                             matches, descriptorType, matcherType, selectorType);

            //// EOF STUDENT ASSIGNMENT

            // store matches in current data frame
            (dataBuffer.end() - 1)->kptMatches = matches;

            cout << "#4 : MATCH KEYPOINT DESCRIPTORS done" <<", Matches = "<<matches.size()<< endl;
            KP_matches_list.push_back(matches.size());

            // visualize matches between current and previous image
            bVis = true;
            if (bVis)
            {
                cv::Mat matchImg = ((dataBuffer.end() - 1)->cameraImg).clone();
                cv::drawMatches((dataBuffer.end() - 2)->cameraImg, (dataBuffer.end() - 2)->keypoints,
                                (dataBuffer.end() - 1)->cameraImg, (dataBuffer.end() - 1)->keypoints,
                                matches, matchImg,
                                cv::Scalar::all(-1), cv::Scalar::all(-1),
                                vector<char>(), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

                string windowName = "Matching keypoints between two camera images";
                cv::namedWindow(windowName, 7);
                cv::imshow(windowName, matchImg);
                cout << "Press key to continue to next image" << endl;
                cv::waitKey(0); // wait for key to be pressed
            }
            bVis = false;
        }
        KP_list.push_back(keypoints.size());
    } // eof loop over all images

    float Total_KP_sum=0.0;
    float detectionTime_sum=0.0;
    float KP_sum=0.0;
    for (int i=0;i<Total_KP_list.size();i++)
    {
        Total_KP_sum+=Total_KP_list[i];
        detectionTime_sum+=detectionTime[i];
        KP_sum+=KP_list[i];
        //cout<<detectionTime[i]<<", ";
    }
    cout<<"\n"<<"Average Total No. of Keypoints per frame = "<<Total_KP_sum/Total_KP_list.size()<<endl;
    cout<<"Average detection time per frame = "<<detectionTime_sum/detectionTime.size()<<" ms"<<endl;
    cout<<"Average No. of Rect Keypoints per frame = "<<KP_sum/KP_list.size()<<endl;

    float descTime_sum=0.0;
    for (int i=0;i<descExtractionTime.size();i++)
    {
        descTime_sum+=descExtractionTime[i];
    }
    cout<<"Average descriptor extraction time = "<<descTime_sum/descExtractionTime.size()<<" ms"<<endl;

    float KP_matches_sum=0.0;
    //cout<<KP_list.size()<<endl;
    for (int i=0;i<KP_matches_list.size();i++)
    {
        KP_matches_sum+=KP_matches_list[i];
    }
    cout<<"Average No. of keypoint matches per frame = "<<KP_matches_sum/KP_matches_list.size()<<endl;


    return 0;
}
