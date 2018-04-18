
//#define MAX_ACTIVE_TURTLES      6
//#define MAX_OPPONENTS           10

typedef struct tag_turtleData
{
        short current_xyo[3];
        char blueIsHomeGoal;
        char teamColor;
        short waypoints_xy[2*8];
        short mergedBall_xyz_xyzdot[6];
        short opponent_xydxdy[4*MAX_OPPONENTS];
        int ms_age;
} turtleData, pturtleData;





typedef struct tag_appData
{
        turtleData td[MAX_ACTIVE_TURTLES];
        short ingame;
        int nclients;
} appData, *pappData;

