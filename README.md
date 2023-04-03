# SFND 2D Feature Tracking

<img src="images/keypoints.png" width="820" height="248" />

The idea of the camera course is to build a collision detection system - that's the overall goal for the Final Project. 

As a preparation for this, I built the feature tracking part and tested various detector / descriptor combinations to see which ones perform best. This mid-term project consists of four parts:

* First, focusing on loading images, setting up data structures and putting everything into a ring buffer to optimize memory load. 
* Then, integrating several keypoint detectors such as HARRIS, FAST, BRISK and SIFT and comparing them with regard to number of keypoints and speed. 
* Focusing on descriptor extraction and matching using brute force and also the FLANN approach that was discussed in the previous lesson. 
* In the last part, testing the various algorithms in different combinations and comparing them with regard to some performance measures. 

<img src="https://raw.githubusercontent.com/hedeya1980/Images/main/2D_feature_tracking.gif" />


	Detector	Total No. of Key points	No. of Keypoints on Preceding Vehicle	Time Taken for Detection (ms)	Outlier keypoints	% of Outlier keypoints	Rect keypoints:Total keypoints											
	SHITOMASI	1342	118	18.0613	45	38%	9%											
	HARRIS	174	25	22.6516	5	20%	14%											
	SIFT	1386	139	138.548	50	36%	10%											
	FAST	1787	149	1.06327	50	34%	8%											
	BRISK	2712	276	372.388	20	7%	10%											
	ORB	500	116	9.87385	10	9%	23%											
	AKAZE	1343	167	107.56	67	40%	12%											
																		
																		
		Descriptors											Descriptors					
No. of Matches		SIFT	BRISK	ORB	FREAK	AKAZE	BRIEF	Average No. of matches	% of matches out of the no. of Rect keypoints		Time Taken for Descriptor Extraction (ms)		SIFT	BRISK	ORB	FREAK	AKAZE	BRIEF
Detectors	SHITOMASI	104	85	101	85	N/A	105	96	81%		Detectors	SHITOMASI	23.0482	331.989	1.19825	49.1395	N/A	1.13875
	HARRIS	19	16	18	16	N/A	19	18	70%			HARRIS	21.9376	332.321	1.05817	47.8131	N/A	0.911672
	SIFT	91	66	OOM	66	N/A	78	75	54%			SIFT	98.1486	373.977	OOM	50.1895	N/A	0.941658
	FAST	125	100	119	98	N/A	122	113	76%			FAST	37.2002	374.522	1.27583	79.3749	N/A	1.08062
	BRISK	188	174	168	169	N/A	189	178	64%			BRISK	77.7862	378.356	7.16449	51.0783	N/A	1.50908
	ORB	86	83	85	47	N/A	61	72	62%			ORB	82.3363	372.796	5.77051	50.3443	N/A	0.883765
	AKAZE	144	135	131	132	140	141	137	82%			AKAZE	40.4469	375.782	3.61785	50.441	101.511	1.32465
![image](https://user-images.githubusercontent.com/20546173/229633356-0e6880ca-1af0-45ad-9e9c-f8031131d8d7.png)



## Dependencies for Running Locally
* cmake >= 2.8
  * All OSes: [click here for installation instructions](https://cmake.org/install/)
* make >= 4.1 (Linux, Mac), 3.81 (Windows)
  * Linux: make is installed by default on most Linux distros
  * Mac: [install Xcode command line tools to get make](https://developer.apple.com/xcode/features/)
  * Windows: [Click here for installation instructions](http://gnuwin32.sourceforge.net/packages/make.htm)
* OpenCV >= 4.1
  * This must be compiled from source using the `-D OPENCV_ENABLE_NONFREE=ON` cmake flag for testing the SIFT and SURF detectors.
  * The OpenCV 4.1.0 source code can be found [here](https://github.com/opencv/opencv/tree/4.1.0)
* gcc/g++ >= 5.4
  * Linux: gcc / g++ is installed by default on most Linux distros
  * Mac: same deal as make - [install Xcode command line tools](https://developer.apple.com/xcode/features/)
  * Windows: recommend using [MinGW](http://www.mingw.org/)

## Basic Build Instructions

1. Clone this repo.
2. Make a build directory in the top level directory: `mkdir build && cd build`
3. Compile: `cmake .. && make`
4. Run it: `./2D_feature_tracking`.
