/*
 * BallShared.h
 *
 */

#ifndef BALL_SHARED_H_
#define BALL_SHARED_H_

/* force optimal alignment */
#pragma pack(push)
#pragma pack(1)

struct BallShared
{
    short mergedBall_xyz_xyzdot[6];       /* merged ball obtained by own sensors (mm and mm/sec) */
    short kinectBall_xyz_xyzdot[6];       /* kinect ball obtained by own sensors (mm and mm/sec) */
    char  mergedBallSource;               /* source of the merged ball (ov,lrf,fc...) */
    char  mergedBallConfidence;           /* confidence of merged ball */
    char  seeBall;                        /* flag for ball observed */
    char  usedBallTurtleID;               /* turtleID of the ball source used in the software */
};

#pragma pack(pop)

#endif /* BALL_SHARED_H_ */
