/*
 * RefboxCommands.h
 *
 */

#ifndef REFBOX_COMMANDS_H_
#define REFBOX_COMMANDS_H_

#include "def_comm.h"

/* force optimal alignment */
#pragma pack(push)
#pragma pack(1)

struct RefboxCommands
{
    char 					teamColor;
    char 					refboxCommand;
    char 					roleId;
    char 					refbox_roleId;
    char 					blueIsHome;
    char 					roleAssignerMode;
    char 					getPhoto;
    char 					getVideo;
    unsigned short 			getVideoNumberOfFrames;
    char 					goalsMagenta;              /* number of goals scored by Magenta */
    char 					goalsCyan;                 /* number of goals scored by Cyan */
    short 					matchTime;
    manual_mode_control_t	controlData; /* manual mode data */
};

#pragma pack(pop)

#endif /* REFBOX_COMMANDS_H_ */
