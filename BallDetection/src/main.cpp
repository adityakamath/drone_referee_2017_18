#include<opencv2/core/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/aruco/charuco.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>
#include <iostream>
#include <ctime>

#include<stdio.h> //printf
#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include<arpa/inet.h> //communication library in linux

//#include<winsock2.h> //communication library in windows 

#include<sys/socket.h> //communication library in linux
#include <errno.h>
//#include "UDPClient.h"

using namespace cv;
using namespace std;



// Changes:
//      5-5-2017 Peter Rooijakkers  initial design
//      9-5-2017 Peter Rooijakkers  add UDP connection
//	19-5-2017 Peter Rooijakkers  add extra markers









// -- SETTINGS ! --
// #define DEBUG false // true to show the feed from the camera, false to hide imshow
#define CAM_DEV_NR 0 // 0 for drone and 1 for laptop because then the webcam is often already 0
#define PARAM_EXPOSURE_TIME 0.01 // keep as low as possible to avoid motion blur

















#define UDP_BUFLEN 512
struct STRUCT_ball_position {
    double x;
    double y;
    double q;
   };



int main(int argc, char* argv[])
{

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
    fs.open("./CameraCalibration_Data.xml", FileStorage::READ);
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
	Mat im_undistort(image_height, image_width, CV_64FC1);

	
	cv::Mat hsvImg;				// HSV Image
	cv::Mat threshImg;			// Thresh Image
	cv::Mat imgThreshLow;
	cv::Mat imgThreshHigh;
	std::vector<cv::Vec3f> v3fCircles;		// 3 element vector of floats, this will be the pass by reference output of HoughCircles()


	int lowH = 21;							// Set Hue
	int highH = 30;

	int lowS = 155;							// Set Saturation
	int highS = 255;

	int lowV = 155;							// Set Value
	int highV = 225;
	// HUE for YELLOW is 21-30.
	// Adjust Saturation and Value depending on the lighting condition of the environment as well as the surface of the object.

// -- UDP_CONNECTION --
    //Structure for address of server
    struct sockaddr_in myaddr;
    myaddr.sin_addr.s_addr = 0; // ip addres INADDR_ANY = local host
    myaddr.sin_port=htons(51234); //port number
    myaddr.sin_family = AF_INET; //http://stackoverflow.com/questions/337422/how-to-udp-broadcast-with-c-in-linux

    // Create Socket
    int sockfd; // file descriptor
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        return 0;
    }

    // -- VARIABLE TO STORE RESULTS --
    STRUCT_ball_position ball_position; // variable to safe position of ball

	VideoCapture cap(cam_dev_nr);		// declare a VideoCapture object to associate webcam, 0 means use 1st (default) webcam

	if (cap.isOpened() == false)	 //  To check if object was associated to webcam successfully
	{
		std::cout << "error: Webcam connect unsuccessful\n";	// if not then print error message
		return(0);												// and exit program
	}


    // -- INITIALIZE VIDEOCAPTURE --
    //VideoCapture cap; // create object
    
	// open camera
    
	//cap.open(cam_dev_nr); // 0 is on my laptop the build in webcam and 1 is external usb cam
    
						  // below: load image/video instead of live video (debug purpose)
    //cap.open("/home/peter/Videos/Webcam/2017-03-30-155118.webm");
    //cap.open("/home/peter/Videos/Webcam/2017-04-06-100945.webm");
    //cap.open("/home/peter/Dropbox/cpp/project_002/sampleImage.jpg");
    //cap.open("/home/peter/Dropbox/cpp/project_002/sampleImage2.jpg");
    //cap.open("/home/peter/Pictures/Webcam/Samples/guvcview_7.jpg");

    // -- CAMERA SETTINGS -- //
    cap.set(CV_CAP_PROP_FRAME_WIDTH, image_width);//set height and width of capture frame
    cap.set(CV_CAP_PROP_FRAME_HEIGHT, image_height);
	
    cap.set(CV_CAP_PROP_FPS, 30);
    cout << "SampleTime " << cap.get(CV_CAP_PROP_FPS) << endl;
    //avoid auto correction  http://stackoverflow.com/questions/13349678/cvcapturefromcam-cvqueryframe-disable-automatic-image-correction

    //cap.set(CAP_PROP_AUTO_EXPOSURE, 0.25); // set to 0.25 to allow manual settings http://answers.opencv.org/question/96137/is-there-any-range-of-values-for-the-exposure-flag/
    //cap.set(CAP_PROP_EXPOSURE, exposure_time); //

    cout << "ExposureTime " << cap.get(CV_CAP_PROP_EXPOSURE) << endl;
    //	cap.set(CV_CAP_PROP_GAIN, 128);
    //	cap.set(CV_CAP_PROP_BRIGHTNESS, 166);
    //	cap.set(CV_CAP_PROP_CONTRAST, 50);
    //	cap.set(CV_CAP_PROP_HUE, 76);
    //  cap.set(CV_CAP_PROP_SATURATION, 128);

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
    while (1)
    {
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
       


      

                // Convert to drone frame (flip x and -y with respect to camera frame)
                //     [ 0 -1  0]
                // V = [ 1  0  0]
                //     [ 0  0  1]
		//imshow("Undistort", frame);
         //      cout << "cameraMatrix = " << endl << " " << cameraMatrix << endl << endl;
	//	cout << "distCoeffs = " << endl << " " << distCoeffs << endl << endl;
		undistort(frame, im_undistort, cameraMatrix, distCoeffs);

			//imshow("Undistort", im_undistort);
       
			cv::cvtColor(im_undistort, hsvImg, CV_BGR2HSV);						// Convert Original Image to HSV Thresh Image
																				
		    cv::inRange(hsvImg, cv::Scalar(0, 155, 155), cv::Scalar(18, 255, 255), imgThreshLow);
			cv::inRange(hsvImg, cv::Scalar(165, 155, 155), cv::Scalar(179, 255, 255), imgThreshHigh);

			cv::add(imgThreshLow, imgThreshHigh, threshImg);
																				
		//	cv::inRange(hsvImg, cv::Scalar(lowH, lowS, lowV), cv::Scalar(highH, highS, highV), threshImg);

			cv::GaussianBlur(threshImg, threshImg, cv::Size(9, 9), 0);			//Blur Effect
																				// Dilate Filter Effect
			cv::erode(threshImg, threshImg, 0);								// Erode Filter Effect
			cv::dilate(threshImg, threshImg, 0);

	// fill circles vector with all circles in processed image
			cv::HoughCircles(threshImg, v3fCircles, CV_HOUGH_GRADIENT, 2, threshImg.rows / 4, 100, 50, 10, 800);  // algorithm for detecting circles		

			for (int i = 0; i < v3fCircles.size(); i++) {						// for each circle
							// radius of circle

																					// draw small green circle at center of object detected
				cv::circle(im_undistort,												// draw on original image
					cv::Point((int)v3fCircles[i][0], (int)v3fCircles[i][1]),		// center point of circle
					3,																// radius of circle in pixels
					cv::Scalar(0, 255, 0),											// draw green
					CV_FILLED);														// thickness

																					// draw red circle around object detected 
				cv::circle(im_undistort,												// draw on original image
					cv::Point((int)v3fCircles[i][0], (int)v3fCircles[i][1]),		// center point of circle
					(int)v3fCircles[i][2],											// radius of circle in pixels
					cv::Scalar(0, 0, 255),											// draw red
					3);

                                v3fCircles[i][0] = (v3fCircles[i][0] - 533) *.581;
                                v3fCircles[i][1] = (377-v3fCircles[i][1]) *.397;
		ball_position.x = v3fCircles[i][0];
		ball_position.y = v3fCircles[i][1];
                ball_position.q = 0;

                // Send to UDP port
                char buf[UDP_BUFLEN]; // Create clear buffer
                memcpy(buf, &ball_position, sizeof(ball_position)); // copy ball_position to buffer
                int bytes_sent = sendto(sockfd, buf, sizeof(ball_position), MSG_DONTWAIT, (struct sockaddr *)&myaddr, sizeof(myaddr)); // send bytes to UDP port
                if(bytes_sent == -1) { printf("Error sending msg: %s\n", strerror(errno)); } // error if bytes not sent	

				std::cout << "Ball position X = " << v3fCircles[i][0]		// x position of center point of circle
					<< ",\tY = " << v3fCircles[i][1]					    // y position of center point of circle
					<< ",\tRadius = " << v3fCircles[i][2] << "\n";

															// thickness
			}

			// declare windows
			cv::namedWindow("frame", CV_WINDOW_AUTOSIZE);
			cv::namedWindow("threshImg", CV_WINDOW_AUTOSIZE);

			/* Create trackbars in "threshImg" window to adjust according to object and environment.*/
			cv::createTrackbar("LowH", "threshImg", &lowH, 179);	//Hue (0 - 179)
			cv::createTrackbar("HighH", "threshImg", &highH, 179);

			cv::createTrackbar("LowS", "threshImg", &lowS, 255);	//Saturation (0 - 255)
			cv::createTrackbar("HighS", "threshImg", &highS, 255);

			cv::createTrackbar("LowV", "threshImg", &lowV, 255);	//Value (0 - 255)
			cv::createTrackbar("HighV", "threshImg", &highV, 255);


			cv::imshow("im_undistort", im_undistort);					// show windows
			cv::imshow("threshImg", threshImg);

			//charCheckForEscKey = cv::waitKey(1);					// delay and get key press
        // Show results on screen
       if(DEBUG) {imshow("Results", im_undistort);}

        // Wait some time to give operating system time to do other stuff
       waitKey(5);

    } // end of while loop
	 
    // Close program
    return 0;
}
