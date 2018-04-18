/*
 * MotionLocal.h
 *
 */

#ifndef MOTION_LOCAL_H_
#define MOTION_LOCAL_H_

/* force optimal alignment */
#pragma pack(push)
#pragma pack(1)

struct MotionLocal
{
    float encoder_position_global_xyo[3];                  /* raw encoder position global [x,y,phi] (m) */
    char  startBackupSoftware;
    short delayed_cur_xyo[3];               /* delayed cur_xyo to l2g-vision time (mm) [-32768 32767]*/
};

#pragma pack(pop)

#endif /* MOTION_LOCAL_H_ */
