/*
 * def_comm.h
 *
 *  Created on: June 1, 2013
 *      Author: KMEESSEN
 *
 */

#ifndef DEF_COMM_H_
#define DEF_COMM_H_

/* definition of bitwise storage of motion status */

typedef union MotionStatus_tag
{
	unsigned char status;
	struct
	{
		unsigned char Joystick       :1;
		unsigned char EMButton       :1;
		unsigned char LowBattery     :1;
		unsigned char CriticalBattery:1;
		unsigned char CapacitorState :1;
		unsigned char RunningBackup  :1;
		unsigned char WaitingBackup  :1;
	};
}MotionStatus_t;

/* The packet loss structure contains the packet loss information for one turtle. packetLoss1 to packetLoss5 contain the
 * percentage of packet loss for information received from peers. Hence, if this structure is sent by Turtle2;
 * packetLoss1 contains the loss of information from Turtle1 to Turtle2
 * packetLoss2 contains the loss of information from Turtle3 to Turtle2
 * packetLoss3 contains the loss of information from Turtle4 to Turtle2
 * etc
 */
typedef union PacketLoss_tag
{
	unsigned int packetLoss;
	struct
	{
		unsigned int packetLoss1	 	:5;
		unsigned int packetLoss2  		:5;
		unsigned int packetLoss3	  	:5;
		unsigned int packetLoss4		:5;
		unsigned int packetLoss5      	:5;
		unsigned int packetLossCoach	:5;
	};
}PacketLoss_t,*pPacketLoss_t;

#define MAX_PACKET_LOSS_VALUE (31.0)		/* 5 bits, hence 2^5 - 1, as a float */

/* The getPhoto structure contains a number that Vision and Motion use to determine whether or not a photo is requested
 * by the TRC.
 * omniPhoto is used for a photo from the omniVision
 * kinectPhoto is used for a photo from the Kinect
 */
typedef union getPhotos_tag
{
	unsigned char getPhoto;
	struct
	{
		unsigned char omniPhoto     :4;
		unsigned char kinectPhoto   :4;
	};
} getPhotos_t;

/* kinectStatus struct contains the status of two kinects. Definition of the value is kinectStatusEnum_t */
typedef union kinectStatus_tag
{
	unsigned char kinectStatus;
	struct
	{
		unsigned char kinect1   :4;
		unsigned char kinect2   :4;
	};
} kinectStatus_t;

typedef enum {
	kinect_inactive = 0,
	kinect_active,
	kinect_sees_ball
} kinectStatusEnum_t;

/* softVersion structure contains the software version info about the turtle. The first bit (key) defines wether the value
 * represents the libraries svn revision or the software revision. */
typedef union softVersion_tag
{
	unsigned short softVersion;
	struct
	{
		unsigned short key		:1;
		unsigned short revision :15;
	};
} softVersion_t;

typedef enum
{
	lib_revision = 0,
	software_revision
} softVersionEnum_t;

/* The communicated skillID contains 4 bits for the skillID and 4 bits for the shotType.
 * The latter contains only valid information if skillID is aim or kick
 */
typedef union skillID_shotType_tag
{
	unsigned char skillID_shotType;
	struct
	{
		unsigned char skillID   :4;
		unsigned char shotType	:4;
	};
} skillID_shotType_t;

typedef union roles_all_turtles_tag
{
	char data[3];
	struct
	{
		char turtle1 :4;
		char turtle2 :4;
		char turtle3 :4;
		char turtle4 :4;
		char turtle5 :4;
		char turtle6 :4;
	};
}roles_all_turtles_t;

typedef struct
{
	char requested_RR	:4;
	char requested_GR	:4;
	char request_need	:4;
	char allow_flag 	:4;
} roleSwitch_t;

#pragma pack(push)
#pragma pack(1)

typedef struct manual_mode_control_tag
{
    char v_x;               /* speed in x direction, -127 is -max speed, 127 is max speed, max speed defined in motion_turtle*/
    char v_y;               /* speed in y direction, -127 is -max speed, 127 is max speed, max speed defined in motion_turtle*/
    char v_phi;             /* speed in phi direction, -127 is -max speed, 127 is max speed, max speed defined in motion_turtle*/
    char command;           /* Manual mode command */
    char kick_effort;       /* 0-100% */
}manual_mode_control_t;

#pragma pack(pop)

#endif /* DEF_COMM_H_ */
