/*
 * TRC2SimLocal.h
 *
 */

#ifndef TRC2SIM_LOCAL_H_
#define TRC2SIM_LOCAL_H_

#include "../../../Global_par/global_par.h"

/* force optimal alignment */
#pragma pack(push)
#pragma pack(1)

struct TRC2SimLocal
{
    short ball_xy_xydot[4];                                     /*ball position and speed in mm and mm/s */
    short turtle_xyo[3*MAX_TURTLES];                            /*turtle position in mm and mrad*/
    char turtle_inout_field[MAX_TURTLES];                       /*turtle in or out of field */
    short obstacle_xyr[3*MAXNOBJ_LOCAL];                        /*obstacle position and radius */
    unsigned char vision_range;                                 /* range of vision */
    unsigned char worldmodel_range;                             /* range of vision */    
    unsigned char ball_noise[MAX_ACTIVE_TURTLES];               /* ball_noise max deviation (mm) */
    char bouncing_borders;                                      /* bouncing border flag */
    unsigned char obs_noise[TRC_NUMBER_OF_TURTLES];				/* noise on the obstacles */
    char obs_offset[TRC_NUMBER_OF_TURTLES];						/* offset on the obstacles */
    unsigned char obs_vy[MAXNOBJ_LOCAL];						/* velocity in y of the obstacles */
    unsigned char obs_dy[MAXNOBJ_LOCAL];						/* displacement of the obstacles */
    unsigned char obs_vx[MAXNOBJ_LOCAL];						/* velocity in y of the obstacles */
    unsigned char obs_dx[MAXNOBJ_LOCAL];						/* displacement of the obstacles */
	char obs_movement_shape[MAXNOBJ_LOCAL];						/* shape of movement of obstacles */
    char autoref_state;                                         /* indicates if AutoRef is activated in the TRC */
    char simulator_link_mode;                                   /* current simulator link mode [host, client, none] */
    char simulator_link_remote_agent;                           /* agent number of other dev PC running simulator link */
};

#pragma pack(pop)

#endif /* TRC2SIM_LOCAL_H_ */
