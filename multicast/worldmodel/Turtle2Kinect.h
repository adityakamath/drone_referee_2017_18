/*
 * Turtle2Kinect.h
 *
 *  Created on: Feb 9, 2016
 *      Author: Koen Meessen
 */

#ifndef TURTLE2KINECT_H_
#define TURTLE2KINECT_H_

#include "../../../Global_par/global_par.h"

/* force optimal alignment */
#pragma pack(push)
#pragma pack(1)

struct Turtle2Kinect
{
	short 	current_xyo[3];
	short	goal_width;
	short	goal_height;
    char    refboxTask;
    unsigned char shotInProgressCnt;                    /* Counter indicating how long ago a shot was started by this turtle */
    float   ball_xyz_ov[MAX_BALLS_TO_KINECT*2];         /* ball position from omnivision */
    float   ball_confidence_ov[MAX_BALLS_TO_KINECT];    /* ball confidence omnivision */
    short   ball_found_ov;                              /* ball found omnivision */
    short   ov_lob_detected;                            /* lob detected [-1,0,1,2] */
    char    collission_counter;                         /* collission counter */
    char 	dummy[11];	/* Adjust the size of this dummy variable such that the total structsize remains the same to avoid changing rtdb.ini */
};

#pragma pack(pop)

#endif /* TURTLE2KINECT_H_ */
