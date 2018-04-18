/*
 * SimLink.h
 *
 */

#ifndef SIMLINK_H_
#define SIMLINK_H_

#include "../../../Global_par/global_par.h"

/* force optimal alignment */
#pragma pack(push)
#pragma pack(1)

struct SimLink
{
    char    simLinkMode;                                /* Simulator link mode [host, client, none] */
    char    blueIsHomeGoal;                             /* Is blue goal my home goal? */
    char    teamColor;                                  /* team color on this pc */
    char    refboxCommand;                              /* Refbox command from host */
    char    has_ball;                                   /* Turtle in ball possession */
    short   ball_xyz[3];                                /*ball position in mm */
    char    turtleIsActive[MAX_ACTIVE_TURTLES];         /* Turtle is active and in-field */
    short   turtlePoses[3*MAX_ACTIVE_TURTLES];          /* Turtle poses [x, y, phi] */
    short   turtleVelocities[2*MAX_ACTIVE_TURTLES];     /* Turtle velocities [x_dot, y_dot] */
    short   FBall[MAX_ACTIVE_TURTLES];                  /* Force to be applied to ball*/
    char    ballAngle[MAX_ACTIVE_TURTLES];              /* Angle of ball when leaving turtle */
    short   obstacle_xy[2*MAXNOBJ_LOCAL];               /*obstacle position in mm */
};

#pragma pack(pop)

#endif /* SIMLINK_H_ */
