/*
 * WorldmodelShared.h
 *
 */

#ifndef WORLDMODEL_SHARED_H_
#define WORLDMODEL_SHARED_H_

#include "../../../Global_par/global_par.h"

/* force optimal byte alignment */
#pragma pack(push)
#pragma pack(1)

struct WorldmodelShared
{
    short opponent_xydxdy[4*MAX_OPPONENTS];    /* worldmodel opponents position */
    char  opponent_label[MAX_OPPONENTS];        /* worldmodel opponents labels [-128 - 127] */
    char  opponentwithball;                     /* opponent that has the ball */
};

#pragma pack(pop)

#endif /* WORLDMODEL_SHARED_H_ */
