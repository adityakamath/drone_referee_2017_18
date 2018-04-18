/*
 * ManualMode.h
 *
 */

#ifndef MANUAL_MODE_H_
#define MANUAL_MODE_H_

#include "def_comm.h"

/* force optimal alignment */
#pragma pack(push)
#pragma pack(1)

struct ManualMode
{
	char 					robotID;			/* id of the turtle to be controlled */
	manual_mode_control_t	controlData;		/* manual mode control data */
};

#pragma pack(pop)

#endif /* MANUAL_MODE_H_ */
