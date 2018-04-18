/*
 * VisionShared.h
 *
 */

#ifndef VISION_SHARED_H_
#define VISION_SHARED_H_

#include "../../../Global_par/global_par.h"

/* force optimal byte alignment */
#pragma pack(push)
#pragma pack(1)

struct VisionShared
{
    char linePoseRecogCompass;                          /* line_pose_recog and compass check */
    char cameraStatus;                                  /* omnivision camera status [-128 127]*/
    char cpu0Load;                                      /* status CPU 0 [-128 127]*/
    char cpu1Load;                                      /* status CPU 1 [-128 127]*/
    short obstacles_xy_for_worldmodel[2*MAXNOBJ_LOCAL]; /* obstacles from omnivision (mm) [-32768 32767]*/
    char compassReadingWrtBlueGoal;                     /* compass reading with respect to blue home (360/255 degrees) [-32768 32767]*/
    unsigned char obstacle_free_angles_to_goal[4];		/* bit array of free angles towards goal */
    unsigned short shuttertime;							/* actual shuttertime */
    unsigned char heartbeat;
    char  dummy[5];										/* not used */
};

#pragma pack(pop)

#endif /* VISION_SHARED_H_ */
