#include <opencv2/highgui.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/aruco/charuco.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>
#include <iostream>
#include <ctime>

#include<stdio.h> //printf
#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include<arpa/inet.h>
#include<sys/socket.h>
#include <errno.h>

using namespace cv;
using namespace std;

// -- SETTINGS ! --
// #define DEBUG false // true to show the feed from the camera, false to hide imshow
#define CAM_DEV_NR 0 // 0 for drone and 1 for laptop because then the webcam is often already 0
#define PARAM_EXPOSURE_TIME 0.01 // keep as low as possible to avoid motion blur

//
//TO DO!!!
//
int players = 4; //2 vs 2
struct STRUCT_player_position{
	double x;
	double y;
    	double z;
    	double yaw;
    	double pitch;
   	double roll;
};

int main(int argc, char* argv[]){

	// -- LOAD INPUT ARGUMENTS --
	cout << "Parameters:"<< endl;
	cout << "  -e 0.01   exposure_time" <<  endl;
	cout << "  -d 1      DEBUG: ( 1 imshow) ( 0 hide)" <<  endl;
	cout << "  -c 0      cam_dev_nr (integer)" << endl;  
	double exposure_time = PARAM_EXPOSURE_TIME;	
	int DEBUG = 0;
	int cam_dev_nr = CAM_DEV_NR;
	for (int i = 1; i < argc; ++i){ // scan through all input arguments 
		if (string(argv[i]) == "-e" && i+1 <= argc){
			exposure_time = atof( argv[i+1]);
			cout << "exposure_time = " << exposure_time << endl;
		}
		if (string(argv[i]) == "-d" && i+1 <= argc){
			DEBUG = atoi( argv[i+1] );
			cout << "DEBUG = "<< DEBUG << endl;
		}
		if (string(argv[i]) == "-c" && i+1 <= argc){
			cam_dev_nr = atoi( argv[i+1] );
			cout << "cam_dev_nr = " << cam_dev_nr << endl;
		}

	}


    	// -- LOAD FROM FILE: Camera Calibration Parameters -- //
    	int image_width, image_height;
    	Mat cameraMatrix, distCoeffs;
    	FileStorage fs("CameraCalibration_Data.xml", FileStorage::READ);
    	fs.open("../CameraCalibration_Data.xml", FileStorage::READ);
    	fs["image_width"] >> image_width;
    	fs["image_height"] >> image_height;
    	fs["camera_matrix"] >> cameraMatrix;
    	fs["distortion_coefficients"] >> distCoeffs;
    	fs.release();

	// -- VARIABLE DECLARATIONS -- //
	// Mat opencv coordinates
	//    --> x
	//  |
	//  v y
	Mat frame; // color
	Mat im; // gray scale
	Mat im_rectangle; //rectangular image from fisheye

    	// -- VARIABLES FOR ARUCO -- //
	//
	//TO DO: make definition
	//
	float marker_length = 0.05;//in meters    	
	Ptr<aruco::Dictionary> aruco_dictionary = aruco::getPredefinedDictionary(aruco::DICT_4X4_250); // load dictionary
    	vector<int> aruco_ids; // to store found marker Id numbers
    	vector<vector<Point2f> > aruco_corners, aruco_rejectedCandidates; // to store found marker corners
	vector<Vec3d> rvecs, tvecs;
	//Mat rvecs(3,3,CV_64F), tvecs(3,1,CV_64F); //marker pose estimation variables (rotation and translation matrices)
	
    	Ptr<aruco::DetectorParameters> aruco_parameters = aruco::DetectorParameters::create(); // set aruco parameters
    	//aruco_parameters->adaptiveThreshWinSizeMin = 100;
    	//aruco_parameters->adaptiveThreshWinSizeMax = 200;
    	//aruco_parameters->adaptiveThreshWinSizeStep = 200;
    	//aruco_parameters->adaptiveThreshConstant = 128;
    	//aruco_parameters->doCornerRefinement = 0;

	//
	//TO DO!!
	//	
	// -- VARIABLE TO STORE RESULTS --
    	//STRUCT_drone_position drone_position; // variable to safe position of drone/camera


    	// -- INITIALIZE VIDEOCAPTURE --
    	VideoCapture cap; // create object
    	// open camera
    	cap.open(cam_dev_nr); 

    	// -- CAMERA SETTINGS -- //
    	cap.set(CV_CAP_PROP_FRAME_WIDTH, image_width);//set height and width of capture frame
    	cap.set(CV_CAP_PROP_FRAME_HEIGHT, image_height);
    	cap.set(CV_CAP_PROP_FPS, 30);
    	cout << "SampleTime " << cap.get(CV_CAP_PROP_FPS) << endl;
    	//avoid auto correction  http://stackoverflow.com/questions/13349678/cvcapturefromcam-cvqueryframe-disable-automatic-image-correction
    	cap.set(CAP_PROP_AUTO_EXPOSURE, 0.25); // set to 0.25 to allow manual settings http://answers.opencv.org/question/96137/is-there-any-range-of-values-for-the-exposure-flag/
    	cap.set(CAP_PROP_EXPOSURE, exposure_time); //
	cout << "ExposureTime " << cap.get(CV_CAP_PROP_EXPOSURE) << endl;
    	//cap.set(CV_CAP_PROP_GAIN, 128);
    	//cap.set(CV_CAP_PROP_BRIGHTNESS, 166);
    	//cap.set(CV_CAP_PROP_CONTRAST, 50);
    	//cap.set(CV_CAP_PROP_HUE, 76);
    	//cap.set(CV_CAP_PROP_SATURATION, 128);
	    // -- Properties of the: ELP Kamera Modul USB 8MP Weitwinkel Fisheye Webcam mit 180 Grad Objektiv -- //
	    //Sensor: Sony (1 / 3.2 ") IMX179
	    //Bildformat : MJPEG / YUY2 /
	    //USB Protocal: USB2.0 HS / FS
	    //Automatische Belichtung AEC: Unterstützung
	    //Auto weiß blance AEB: Unterstützung
	    //Effektive Pixel:
	    //3264X2448/3200X2400/2592X1944/
	    //2048X1536/1600X1200/1280X960/
	    //1024X768/800X600/640X480
	    //(andere spezielle Größe Auflösung angepasst)
	    //Performance:
	    //3264X2448 MJPEG 15fps YUY2 2fps/3200X2400 MJPEG 30fps YUY2 30fps
	    //2592X1944 MJPEG 15fps YUY2 3fps/2048X1536 MJPEG 20fps YUY2 3fps
	    //1600X1200 MJPEG 20fps YUY2 10fps/1280X960 MJPEG 20fps YUY2 10fps
	    //1024X768 MJPEG 30fps YUY2 10fps/800X600 MJPEG 30fps YUY2 30fps
	    //640X480 MJPEG 30fps YUY2 30fp

    	// check if we succeeded to open device
   	if (!cap.isOpened()) {
        	cerr << "ERROR! Unable to open camera\n";
        	return -1;
    	}

    	// -- INFINIT LOOP --
    	cout << "Start grabbing" << endl;
    	clock_t timer_begin = clock(); // timer to measure execution time
    	double timer_elapsed_time;
	while (1){
        	// get new frame from cam
       		cap.read(frame);
        	if (frame.empty()) {
        		cerr << "ERROR! blank frame grabbed\n";
            		break;
        	}

        
		// Show sample time
        	//if(DEBUG) {
            		timer_elapsed_time = double(clock() - timer_begin) / CLOCKS_PER_SEC;
            		//cout << "Tsample: "<< timer_elapsed_time << endl;
            		timer_begin = clock();
        	//}

        	// Color image to gray scale
        	cvtColor(frame,im,COLOR_RGB2GRAY);

        	// Check if camera calibration worked
		// undistort(frame, im_rectangle, cameraMatrix, distCoeffs);
  		// imshow("Undistort", im_rectangle);


	//
	//TO DO?
	//        
	// Remove old found markers from vector
        //aruco_ids.clear();
        //aruco_corners.clear();

        	// Detect the markers
        	cv::aruco::detectMarkers(im, aruco_dictionary, aruco_corners, aruco_ids, aruco_parameters, aruco_rejectedCandidates);
        	if(DEBUG) { cv::aruco::drawDetectedMarkers(frame, aruco_corners, aruco_ids);} // show results graphically

		// Track the markers
		if(aruco_ids.size() > 1){
			cv::aruco::estimatePoseSingleMarkers(aruco_corners, marker_length, cameraMatrix, distCoeffs, rvecs, tvecs);
		}        	
		if(DEBUG) { cv::aruco::drawAxis(frame, cameraMatrix, distCoeffs, rvecs, tvecs, 0.1);} // show results graphically
	        
		// Show results on screen
        	if(DEBUG) {imshow("Results", frame);}
        	// Wait some time to give operating system time to do other stuff
        	waitKey(5);

	} // end of while loop

	// Close program
    	return 0;
}
