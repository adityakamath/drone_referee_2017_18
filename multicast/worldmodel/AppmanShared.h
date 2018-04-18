/*
 * AppmanShared.h
 *
 */

#ifndef APPMAN_SHARED_H_
#define APPMAN_SHARED_H_

/* force optimal alignment */
#pragma pack(push)
#pragma pack(1)

struct AppmanShared
{
    short restartCountMotion;             /* restart counter motion [-32768 32767]*/
    short restartCountVision;             /* restart counter vision [-32768 32767]*/
    short restartCountWorldmodel;         /* restart counter worldmodel [-32768 32767]*/
};

#pragma pack(pop)

#endif /* APPMAN_SHARED_H_ */
