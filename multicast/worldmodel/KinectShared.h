/*
 * KinectShared.h
 *
 */

#ifndef KINECT_SHARED_H_
#define KINECT_SHARED_H_

#include "../../../Global_par/global_par.h"

/* force optimal alignment */
#pragma pack(push)
#pragma pack(1)

struct KinectShared
{
    short Ball_xyz[3*MAXBALLS_SINGLE_KINECT];	        /* merged ball obtained by own sensors (mm and mm/sec) */
    char  Confidence[MAXBALLS_SINGLE_KINECT];	        /* confidence of merged ball */
    short Obstacle_distance[KINECT_GOALSCAN_LENGTH];    /* Array of distances to local obstacles */
    double timestamp;							        /* image timestamp */
};

#pragma pack(pop)

#endif /* Kinect_SHARED_H_ */
