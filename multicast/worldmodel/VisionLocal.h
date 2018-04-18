/*
 * VisionLocal.h
 *
 */

#ifndef VISION_LOCAL_H_
#define VISION_LOCAL_H_

#include "../../../Global_par/global_par.h"

/* force optimal alignment */
#pragma pack(push)
#pragma pack(1)

struct VisionLocal
{
    float pose_xyo_omnivision[3];                               /* pose omnivision [x,y,phi] */
    int pose_omnivision_found;                                  /* confidence pose_vision_found */
    float ball_xyz_omnivision[3*MAXBALLS_OV];                   /* ball position */
    float ball_confidence_omnivision[MAXBALLS_OV];              /* ball confidence */
    short ball_found_omnivision;                                /* ball found */    
    float obstacles_xyxydotr_for_strategy[5*NAVOIDABLES];       /* obstacles from omnivision */ 
    double delay_times[2];                                      /* image time, l2g-vision time */
    int lob_detected;                                           /* lob detected? [-1,0,1,2] */
    unsigned char obstacle_free_angles_to_goal[13];  			/* bit array of free angles towards goal */
    char dummy[23];												/* unused */
};

#pragma pack(pop)

#endif /* VISION_LOCAL_H_ */
