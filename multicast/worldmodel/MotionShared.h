/*
 * MotionShared.h
 *
 */

#ifndef MOTION_SHARED_H_
#define MOTION_SHARED_H_

#include "def_comm.h"

/* force optimal byte alignment */
#pragma pack(push)
#pragma pack(1)

struct MotionShared
{
    short current_xyo[3];						/* current robot position [x,y,phi] (mm) [-32768 32767] */
    short cpb_poi_xy[2];                    	/* lastest ball catch location [x,y] (mm) [-32768 32767] */
    short current_xy_dot[3];                	/* current robot velocity [x dot,y dot] (mm/s) [-32768 32767] */
    short target_xyo[3];                    	/* target position [x,y,phi] (mm) [-32768 32767]*/
    short gameRoleTarget_xy[2];             	/* game role target position [x,y] (mm) [-32768 32767]*/
    char  gameRoleTarget_Role;          		/* GameRole at same timestamp as gameRoleTarget_xy */
    short refboxRoleTarget_xy[2];           	/* refbox role target position [x,y] (mm) [-32768 32767] and RefboxRole*/
    char  refboxRoleTarget_Role;            	/* RefboxRole at same timestamp as refboxRoleTarget_xy */
    short subtarget_xy[2];                  	/* subtarget position [x,y] (mm) [-32768 32767]*/
    char robotInField;                      	/* robot I/O active and enabled, in field */
    char roleId[2];                         	/* [current role Id, previous role Id] */
    char refboxRole;                        	/* refbox role */
    char defaultRole;                       	/* default role */
    roleSwitch_t roleSwitch;          			/* [role switch enable, role switching team member] */
    skillID_shotType_t skill_and_shottype;     	/* skill Id */
    char blueIsHomeGoal;                    	/* blue is home goal */
    char teamColor;                         	/* team color */
    char CPPArobot;                         	/* CPPA robot */
    char CPBrobot;                          	/* CPB robot */
    char CPBteam;                           	/* team is in possession of ball */
    char batteryVoltage;                    	/* battery voltage (in percent!)*/
    char emergencyStatus;                   	/* emergency status */
    MotionStatus_t motionStatus;                /* def_comm.h */
    short refboxStageState;                 	/* stage state during refbox task [-32768 32767]*/
    short RoleStageState;                   	/* stage state while in game roles [-32768 32767]*/
    char  refboxStatus; 						/* refbox status (enum RefboxStatus_t) */
    unsigned char refboxVariant;               	/* refbox variant (see refboxhandler) */
    unsigned char refboxRegionBitmask;          /* refbox region bitmask (RefboxRegionBitmask_t) */
    char refboxRandomNumber;					/* refbox randomnumber */
    char  Pass_receiver_ID;						/* id for pass receiver. Turtle for in-game, role id for refbox passing. */
    short Pass_target_xy[2];                	/* target to pass to [x,y] */
    unsigned char motor_temperature[3];     	/* temperature of drive motors (degrees)*/
    softVersion_t softVersion;                  /* software version and library version (see def_comm.h for definition of structure) */
    kinectStatus_t kinectStatus;				/* status of the kinect */
    char poseLRF_found;                     	/* pose found using laserrangefinder of keeper */
    PacketLoss_t packetLoss;               		/* def_comm.h */
    char action_ID;								/* Action ID */
    char muFieldNr;								/* Number of the MuField that is currently being calculated */
    roles_all_turtles_t assigned_refbox_roles;	/* Game roles of all turtles calculated by role_assigner of this robot */
    roles_all_turtles_t assigned_game_roles;	/* Refbox roles of all turtles calculated by role_assigner of this robot */
    unsigned char shotInProgressCnt;		    /* Counter indicating how long ago a shot was started by this turtle */
    unsigned char shotDutyCycle;				/* dutycycle of the shot */
    unsigned char defconSituation;				/* defcon situation value */
    unsigned char LEDs;							/* placeholder for current led-colors */
    char STP_TaskExecutionStatus; 			    /* Current progress of the executed task of the executed play */
    char STP_SelectedPlay;					    /* Selected play by a turtle */
    char STP_PlayState;						    /* Current Play state identified by this turtle */
    char WMOpponentsToDefendLabel_Refbox[5]; 	/* Labels of the opponents to defend [R1/R2/R3/R4/R5] */
    char WMOpponentsToDefendLabel_Game[5];		/* Labels of the opponents to defend [GK/AM/AA/DM/DA1/DA2] IMPLEMENT AFTER TOURNAMENT JAPAN*/
    unsigned char dummy[1];						/* REDUCE THE SIZE OF THIS DUMMY WHEN ADDING VARIABLES TO MAKE SURE THE STRUCT SIZE DOES NOT CHANGE */
};

#pragma pack(pop)

#endif /* MOTION_SHARED_H_ */
