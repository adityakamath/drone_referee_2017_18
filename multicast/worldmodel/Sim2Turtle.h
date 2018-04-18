/*
 * Sim2Turtle.h
 *
 */

#ifndef SIM2TURTLE_H_
#define SIM2TURTLE_H_

#include "../../../Global_par/global_par.h"

/* force optimal alignment */
#pragma pack(push)
#pragma pack(1)

struct Sim2Turtle
{
    char has_ball;                                  /*bot_id of turtle which has ball */
    short ball_xyz[3];                              /*ball position in mm */
    short turtle_xyo[3*MAX_ACTIVE_TURTLES];         /*turtle position in mm and mrad */
    short obstacle_xy[2*MAXNOBJ_LOCAL];             /*obstacle position in mm */
    char turtle_inout_field[MAX_ACTIVE_TURTLES];    /*turtle in or out of field */
    unsigned char vision_range;                     /* range of vision */
    unsigned char worldmodel_range;                 /* range of worldmodel */
    unsigned char ball_noise[TRC_NUMBER_OF_TURTLES];   /* ball_noise max deviation (cm) */
    unsigned char obs_noise[TRC_NUMBER_OF_TURTLES];	/* noise on the obstacles (pixels) */
    char obs_offset[TRC_NUMBER_OF_TURTLES];			/* offset on the obstacles (pixels) */
    char autoref_command;                           /* Refbox command created by AutoRef */
};

#pragma pack(pop)

#endif /* SIM2TURTLE_H_ */
