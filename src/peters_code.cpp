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
#include<arpa/inet.h>
#include<sys/socket.h>
#include <errno.h>

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

struct STRUCT_drone_position {
    double x;
    double y;
    double z;
    double yaw;
    double pitch;
    double roll;
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


    // -- VARIABLES FOR ARUCO -- //
    Ptr<aruco::Dictionary> aruco_dictionary = aruco::getPredefinedDictionary(aruco::DICT_4X4_250); // load dictionary
    vector<int> aruco_ids; // to store found marker Id numbers
    vector<vector<Point2f> > aruco_corners, aruco_rejectedCandidates; // to store found marker corners


    Ptr<aruco::DetectorParameters> aruco_parameters = aruco::DetectorParameters::create(); // set aruco parameters
    //aruco_parameters->adaptiveThreshWinSizeMin = 100;
    //aruco_parameters->adaptiveThreshWinSizeMax = 200;
    //aruco_parameters->adaptiveThreshWinSizeStep = 200;
    //aruco_parameters->adaptiveThreshConstant = 128;
    //aruco_parameters->doCornerRefinement = 0;




    // World-points/object-points of Aruco markers currently hardcoded
    // aruco corner coordinates (clock wise)
    //  1 2
    //  3 4

    // Creates a vector of vectors where each element is a 3D position of one of the corners of the aruco markers
    // VectorWithMarkers.VectorWith4MarkerCorners.3DPosOf1Corner
    float m = 0.095; // Marker size in meter (rib length of black square)
    float ox = 0.210; // Ofset of markers in x-dir
    float oy = 0.297; // Ofset of markers in y-dir

    vector<int> aruco_objIds; // store ID marker WF (world frame)
    vector< vector<Point3f> > aruco_objPoints; // store all corner pos WF
    vector<Point3f> aruco_objPoints_temp; // store corners of 1 marker temprary
     //  Hard coded world points
    int idn = 100; // first marker ID
    for(int i = 0; i < 4; ++i){ // i markers in X direction
        for(int j = 0; j < 4; ++j){ // j markers in Y direction
            aruco_objIds.push_back(idn); // add ID to vector
            aruco_objPoints_temp.clear(); // clear temporary vector
            aruco_objPoints_temp.push_back( Point3f(i*ox+0,j*oy+0,0) ); // add corner 1 to temporary vector
            aruco_objPoints_temp.push_back( Point3f(i*ox+m,j*oy+0,0) ); // add corner 2 to temporary vector
            aruco_objPoints_temp.push_back( Point3f(i*ox+m,j*oy+m,0) ); // add corner 3 to temporary vector
            aruco_objPoints_temp.push_back( Point3f(i*ox+0,j*oy+m,0) ); // add corner 4 to temporary vector
            aruco_objPoints.push_back( aruco_objPoints_temp ); // add temporary vector to vector with all corners
            ++idn; // increase aruco ID
        }
    }


   // Second orgin of markers
	m = 0.123;
	ox = 0.75;
	oy = 0.75;
	idn = 50;
        for(int i = 0; i < 4; ++i){ // i markers in X direction
        	for(int j = 0; j < 5; ++j){ // j markers in Y direction
            		aruco_objIds.push_back(idn); // add ID to vector
           		aruco_objPoints_temp.clear(); // clear temporary vector
            		aruco_objPoints_temp.push_back( Point3f(i*ox+0,j*oy+0,0) ); // add corner 1 to temporary vector
            		aruco_objPoints_temp.push_back( Point3f(i*ox+m,j*oy+0,0) ); // add corner 2 to temporary vector
            		aruco_objPoints_temp.push_back( Point3f(i*ox+m,j*oy+m,0) ); // add corner 3 to temporary vector
            		aruco_objPoints_temp.push_back( Point3f(i*ox+0,j*oy+m,0) ); // add corner 4 to temporary vector
            		aruco_objPoints.push_back( aruco_objPoints_temp ); // add temporary vector to vector with all corners
            		++idn; // increase aruco ID
        	}
    	}
 

    // -- UDP_CONNECTION --
    //Structure for address of server
    struct sockaddr_in myaddr;
    myaddr.sin_addr.s_addr=0;//htonl(INADDR_ANY); // ip addres INADDR_ANY = local host
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
    STRUCT_drone_position drone_position; // variable to safe position of drone/camera


    // -- INITIALIZE VIDEOCAPTURE --
    VideoCapture cap; // create object
    // open camera
    cap.open(cam_dev_nr); // 0 is on my laptop the build in webcam and 1 is external usb cam
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

    cap.set(CAP_PROP_AUTO_EXPOSURE, 0.25); // set to 0.25 to allow manual settings http://answers.opencv.org/question/96137/is-there-any-range-of-values-for-the-exposure-flag/
    cap.set(CAP_PROP_EXPOSURE, exposure_time); //

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
        //undistort(frame, im, cameraMatrix, distCoeffs);
        //imshow("Undistort", im);

        // Remove old found markers from vector
        aruco_ids.clear();
        aruco_corners.clear();

        // Detect the markers
        detectMarkers(im, aruco_dictionary, aruco_corners, aruco_ids, aruco_parameters, aruco_rejectedCandidates);
        if(DEBUG) { cv::aruco::drawDetectedMarkers(frame, aruco_corners, aruco_ids);} // show results graphically


        // -- MARKER DETECTED --//
        //if at least one marker detected
        if (aruco_ids.size() > 0) {

            // Check if the markers occure in our data base
            // if so couple 2D image data points to 3D worldframe data points
            vector<Point2f> aruco_2D; // new vector with 2D image points
            vector<Point3f> aruco_3D; // new vector with 3D world/object points
            for (int k = 0; k < aruco_ids.size(); ++k){ // for all detected markers
                for (int n = 0; n < aruco_objIds.size(); ++n){ // walk through our database
                    if (aruco_ids[k] == aruco_objIds[n]){ // if marker in database
                        for (int c = 0; c < 4; ++c){ // copy all 4 aruco markers
                            aruco_2D.push_back(aruco_corners[k][c]); // add to vector
                            aruco_3D.push_back(aruco_objPoints[n][c]);// add to vector
                        }
                        break; // if found in database jump to next found marker
                    }
                }
            }

            //If there are markers that match the database
            if (aruco_2D.size() > 1) {

                // solvePnP
                Mat aruco_rvec, aruco_tvec;
                solvePnP( aruco_3D, aruco_2D, cameraMatrix, distCoeffs, aruco_rvec, aruco_tvec, false, SOLVEPNP_ITERATIVE );

                // Convert to drone frame (flip x and -y with respect to camera frame)
                //     [ 0 -1  0]
                // V = [ 1  0  0]
                //     [ 0  0  1]
                //Mat V = (Mat_<double>(3,3) << 0, -1, 0, 1, 0, 0, 0, 0, 1);
		Mat V = (Mat_<double>(3,3) << 1, 0, 0, 0, 1, 0, 0, 0, 1);
                Mat aruco_rvec_drone = V*aruco_rvec;
                Mat aruco_tvec_drone = V*aruco_tvec;

                // GET Postion
                // express camera position in world frame
                Mat aruco_rotationMatrix(3,3,CV_64FC1), aruco_tvec_droneWF(3,1,CV_64FC1); // variable declaration of type double
                Rodrigues(aruco_rvec_drone, aruco_rotationMatrix); // Rodrigues to rotation matrix
                //aruco_rotationMatrix = V*aruco_rotationMatrix;
                //Mat aruco_translationVector = V*tvecWF;
                Mat aruco_rotationMatrix_inv = aruco_rotationMatrix.t(); // Inverse of rotation matrix is equal to transpose
                aruco_tvec_droneWF = aruco_rotationMatrix_inv*(aruco_tvec_drone).mul(-1.0); // position of camera expressed in world frame tWF = -R^T*tCF

                drone_position.x = aruco_tvec_droneWF.at<double>(0,0);
                drone_position.y = aruco_tvec_droneWF.at<double>(1,0);
                drone_position.z = aruco_tvec_droneWF.at<double>(2,0);


                // GET Roll, Pitch, Yaw
                // Select some elements from the rotation matrix (always difficult in opencv)
                // Some info about how to acces elements in a Mat
                // CV_8U then use Mat.at<uchar>(y,x).
                // CV_8S then use Mat.at<schar>(y,x).
                // CV_16U then use Mat.at<ushort>(y,x).
                // CV_16S then use Mat.at<short>(y,x).
                // CV_32S then use Mat.at<int>(y,x).
                // CV_32F then use Mat.at<float>(y,x).
                // CV_64F then use Mat.at<double>(y,x).
                double r00 = aruco_rotationMatrix_inv.at<double>(0,0);
                double r10 = aruco_rotationMatrix_inv.at<double>(1,0);
                double r21 = aruco_rotationMatrix_inv.at<double>(2,1);
                double r22 = aruco_rotationMatrix_inv.at<double>(2,2);
                double r20 = aruco_rotationMatrix_inv.at<double>(2,0);
                // roll pitch yaw from rotation matrix http://planning.cs.uiuc.edu/node103.html
                drone_position.yaw = atan2( r10, r00);
                drone_position.pitch = atan2( -1.0*r20, sqrt( r21*r21 + r22*r22));
                drone_position.roll = atan2( r21, r22);


                // Send to UDP port
                char buf[UDP_BUFLEN]; // Create clear buffer
                memcpy(buf, &drone_position, sizeof(drone_position)); // copy drone_position to buffer
                int bytes_sent = sendto(sockfd, buf, sizeof(drone_position), MSG_DONTWAIT, (struct sockaddr *)&myaddr, sizeof(myaddr)); // send bytes to UDP port
                if(bytes_sent == -1) { printf("Error sending msg: %s\n", strerror(errno)); } // error if bytes not sent

                // Print results on terminal
                cout <<"Bytes sent: "<< bytes_sent << " @ "<< 1/timer_elapsed_time <<" Hz"<< " x"<< drone_position.x << " y"<< drone_position.y << " z"<< drone_position.z << " roll"<< drone_position.roll << " pitch"<< drone_position.pitch << " yaw"<< drone_position.yaw << endl;

                // Show results on image
                if(DEBUG) {
                    // show axis
                    aruco::drawAxis(frame, cameraMatrix, distCoeffs, aruco_rvec, aruco_tvec, 0.5);
                    // Show text on image
                    char str[200];
                    sprintf(str,"X %f", drone_position.x);
                    putText(frame, str, Point2f(20,20), FONT_HERSHEY_TRIPLEX, 0.8,  Scalar(0,0,255,0));
                    sprintf(str,"Y %f", drone_position.y);
                    putText(frame, str, Point2f(20,50), FONT_HERSHEY_TRIPLEX, 0.8,  Scalar(0,0,255,0));
                    sprintf(str,"Z %f", drone_position.z);
                    putText(frame, str, Point2f(20,80), FONT_HERSHEY_TRIPLEX, 0.8,  Scalar(0,0,255,0));
                    sprintf(str,"Roll %f", drone_position.roll);
                    putText(frame, str, Point2f(20,110), FONT_HERSHEY_TRIPLEX, 0.8,  Scalar(0,0,255,0));
                    sprintf(str,"Pitch %f", drone_position.pitch);
                    putText(frame, str, Point2f(20,140), FONT_HERSHEY_TRIPLEX, 0.8,  Scalar(0,0,255,0));
                    sprintf(str,"Yaw %f", drone_position.yaw);
                    putText(frame, str, Point2f(20,170), FONT_HERSHEY_TRIPLEX, 0.8,  Scalar(0,0,255,0));

                    sprintf(str,"%f Hz", 1.0/(timer_elapsed_time+0.00001));
                    putText(frame, str, Point2f(20,220), FONT_HERSHEY_TRIPLEX, 0.8,  Scalar(0,255,0,0));

                    sprintf(str,"Orthogonal frame at image borders may be incorrect because of undistored image.");
                    putText(frame, str, Point2f(20,image_height-10), FONT_HERSHEY_PLAIN, 1,  Scalar(255,255,255,0));
                }
            }
        }

        // Show results on screen
        if(DEBUG) {imshow("Results", frame);}
        // Wait some time to give operating system time to do other stuff
        waitKey(5);

    } // end of while loop

    // Close program
    return 0;
}
