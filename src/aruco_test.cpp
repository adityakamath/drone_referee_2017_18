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

int main(int argc, char* argv[]){
	cv::VideoCapture inputVideo;
	inputVideo.open(0);

	int image_width, image_height;
	Mat cameraMatrix, distCoeffs;
	FileStorage fs("CameraCalibration_Data.xml", FileStorage::READ);
	fs.open("../CameraCalibration_Data.xml", FileStorage::READ);
	fs["image_width"] >> image_width;
	fs["image_height"] >> image_height;
	fs["camera_matrix"] >> cameraMatrix;
	fs["distortion_coefficients"] >> distCoeffs;
	fs.release();

	Ptr<aruco::Dictionary> aruco_dictionary = aruco::getPredefinedDictionary(aruco::DICT_4X4_250); // load dictionary

	while(inputVideo.grab()){
		Mat image, imageCopy;
		inputVideo.retrieve(image);
		image.copyTo(imageCopy);

		vector<int> ids;
		vector<vector<Point2f> > corners;
		aruco::detectMarkers(image, aruco_dictionary, corners, ids);

		if(ids.size() > 0){
			aruco::drawDetectedMarkers(imageCopy, corners, ids);

			vector<Mat> rvecs, tvecs;
			aruco::estimatePoseSingleMarkers(corners, 0.05, cameraMatrix, distCoeffs, rvecs, tvecs);
			for(int i=0; i<ids.size(); i++){
				aruco::drawAxis(imageCopy, cameraMatrix, distCoeffs, rvecs[i], tvecs[i], 0.1);
			}
		}

	imshow("out", imageCopy);
}	
return 0;
}

