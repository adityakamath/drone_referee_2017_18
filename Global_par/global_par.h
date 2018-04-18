#ifndef GLOBAL_PAR_LIST
#define GLOBAL_PAR_LIST

/*
    global_par.h

    global parameters for techunited software

    IMPORTANT: parameters should be defined as scalar numbers!

    c-programmers: include this file in your code
    m-programmers: use get_global_par.m to access parameters from this file

    Rene van de Molengraft, November, 10th, 2006

    Last updated: Roel Merry, April 10, 2007, Switch to field sizes for 2007: #define largefield
                  WA/RvdM, April 14, 2007, changed to real refbox values
                  RvdM, March, 20th, 2008, added soccer roles
*/

#define MAXNOBJ_LOCAL               10      /* as tracked by local tracker */
#define MAXNOBJ_GLOBAL              12      /* as tracked by global tracker */
#define NAVOIDABLES	                50      /* maximum amount of avoidables that can be found by vision */

#define NPATH_MAX                   50      /* maximum length of the global path (waypoints) */
#define NOBST_TEAMMEMBERS_MAX       5       /* maximum number of obstacles per path of team members */

#define MAX_TURTLES                 7
#define TRC_NUMBER_OF_TURTLES       6
#define MAX_ACTIVE_TURTLES          6
#define MAX_OPPONENTS               10      /* maximum number of opponents in worldmodel */
#define MAX_OBSTACLES               48      /* maximum number of merged obstacles */
#define MAX_ADDEDOBSTACLES			30		/* maximum number of obstacles that can be added */
#define MAXBALLS_OV                 10      /* maximum number of candidate balls found by omnivision and send to tracker */
#define MAXBALLS_LRF                10      /* maximum number of candidate balls found by LRF-field and send to tracker*/
#define MAX_BALLS_TO_KINECT         3       /* maximum number of candidate balls found by omnivision that are send to the Jetson */
#define MAXBALLS_SINGLE_KINECT		5		/* maximum number of candidate balls found per kinect */
#define MAX_JETSONS                 2
#define MAXBALLS_KINECT				10		/* this should be MAXBALLS_SINGLE_KINECT*MAX_JETSONS, which cannot be handled by get_global_par */
#define KINECT_GOALSCAN_LENGTH      256

#define MAX_DEVPCS                  20

/* refbox commands (as produced by the actual refbox computer and the refbox simulator) */
#define COMM_STOP                   'S'
#define COMM_START                  's'

/* general game flow commands */
#define COMM_START_FIRST_HALF       '1'
#define COMM_HALF_TIME              'h'
#define COMM_START_SECOND_HALF      '2'
#define COMM_END_GAME               'e'

#define COMM_PARKING                'L'
#define COMM_CANCEL                 'x'

#define COMM_GOAL_MAGENTA           'a'
#define COMM_GOAL_CYAN              'A'
#define COMM_WITHDRAW_GOAL_MAGENTA  'd'
#define COMM_WITHDRAW_GOAL_CYAN  	'D'
/* refbox commands */

#define COMM_KICKOFF_MAGENTA        'k'
#define COMM_KICKOFF_CYAN           'K'

#define COMM_FREEKICK_MAGENTA       'f'
#define COMM_FREEKICK_CYAN          'F'

#define COMM_GOALKICK_MAGENTA       'g'
#define COMM_GOALKICK_CYAN          'G'

#define COMM_THROWIN_MAGENTA        't'
#define COMM_THROWIN_CYAN           'T'

#define COMM_CORNER_MAGENTA         'c'
#define COMM_CORNER_CYAN            'C'

#define COMM_PENALTY_MAGENTA        'p'
#define COMM_PENALTY_CYAN           'P'

#define COMM_DROPPED_BALL           'N'

#define COMM_REPAIR_OUT_MAGENTA     'o'
#define COMM_REPAIR_OUT_CYAN        'O'
#define COMM_REPAIR_IN_MAGENTA      'i'
#define COMM_REPAIR_IN_CYAN         'I'
#define COMM_RESET         			'Z'
#define COMM_WELCOME        		'W'
#define COMM_YELLOW_CARD_CYAN		'Y'
#define COMM_2NDYELLOW_CARD_CYAN	'B'
#define COMM_RED_CARD_CYAN			'R'
#define COMM_YELLOW_CARD_MAGENTA	'y'
#define COMM_2NDYELLOW_CARD_MAGENTA	'b'
#define COMM_RED_CARD_MAGENTA		'r'

/* refbox commands (as handled by refbox handler) */
#define COMM_KICKOFF_ACTIVE         30
#define COMM_FREEKICK_ACTIVE        31
#define COMM_GOALKICK_ACTIVE        32
#define COMM_THROWIN_ACTIVE         33
#define COMM_CORNER_ACTIVE          34
#define COMM_PENALTY_ACTIVE         35

#define COMM_DROPPED_BALL_TE        36
#define COMM_START_TE               37
#define COMM_STOP_TE                38

#define COMM_KICKOFF_PASSIVE        39
#define COMM_FREEKICK_PASSIVE       40
#define COMM_GOALKICK_PASSIVE       41
#define COMM_THROWIN_PASSIVE        42
#define COMM_CORNER_PASSIVE         43
#define COMM_PENALTY_PASSIVE        44

#define COMM_PARKING_TE             45
#define COMM_GO_IN_FIELD_TE         46

#define COMM_START_FIRST_HALF_TE    47
#define COMM_START_SECOND_HALF_TE   48

/*define role assigner modes */
#define ROLE_ASSIGNER_OFF       0
#define ROLE_ASSIGNER_ON        1
#define DEMO_ROLES              2
#define PENALTY_DEMO            3
#define BALLHANDLING_DEMO       4
#define PASS_DEMO               5
#define PASS_CALIBRATION        6
#define TECH_CHALL              7

/* soccer role definition */
#define MAX_GAME_ROLES              7

#define ROLE_NONE                   -1
#define ROLE_ATTACKER_MAIN          1
#define ROLE_ATTACKER_ASSIST        2
#define ROLE_DEFENDER_MAIN          3
#define ROLE_DEFENDER_ASSIST1       4
#define ROLE_DEFENDER_ASSIST2       5
#define ROLE_GOALKEEPER             6
#define MAX_ROLES                   7   /* total number of roles in 'role_handler' mode */

#define ROLE_AB1        101
#define ROLE_AB2        102 
#define ROLE_AB3        103
#define N_AB_ROLES      3

#define ROLE_TC1        201

/* refbox role definitions */
#define MAX_REFBOX_ROLES            6
#define ROLE_REFBOX_NONE            -1
#define ROLE_REFBOX_1               1
#define ROLE_REFBOX_2               2
#define ROLE_REFBOX_3               3
#define ROLE_REFBOX_4               4
#define ROLE_REFBOX_5               5

/* play role definitions */
#define PLAY_ROLE_1 				(1)
#define PLAY_ROLE_2 				(2)
#define PLAY_ROLE_3 				(3)
#define PLAY_ROLE_4					(4)
#define PLAY_ROLE_5					(5)
#define PLAY_ROLE_6					(6)

/* For mu positioning */
#define MAX_PEERS                   6

#define DESIRED_GRIDRES             0.25    /* Passing grid resolution [meter] */

#define MINTHRESH_PASS              0.8     /* The minimum probability MU should have at a certain point if one would like to pass there */
#define MAXNOPT_MU                  5       /* The maximum number of local optima you want to detect in your mu-field */

#define NPASS_PEERS                 4       /* Define number of peer TURTLES joining in pass (in order of AM, AA, DM,DA1..)*/
#define MIN_DIST_T2PT_PASS          5.0     /* Minimal distance intercepting-turtle to pass_target when passing [m] */
#define NMU                         16      /* Total number of membership functions (extra 1 for total fields) */
#define ANGLE_DELTA_MUPOS			12.0*M_PI/180.0 /*Angle to compute negative trapezoid field behind opponents*/

/* MSL Refbox Rules (MSLRR) 2011 */
#define MSLRR_MIN_DIST_TO_BALL_REFBOX_PASSIVE                       3.0
#define MSLRR_MIN_DIST_TO_BALL_REFBOX_PASSIVE_SUBTARGET_OBSTACLE    1.0
#define MSLRR_MIN_DIST_TO_BALL_REFBOX_ACTIVE                        2.0
#define MSLRR_MIN_DIST_TO_BALL_REFBOX_ACTIVE_SUBTARGET_OBSTACLE     1.0
#define MSLRR_MIN_DIST_TO_BALL_DROPPED_BALL							1.0

/* Tunable parameter defines */
#define MAX_TUNABLE_PARS            100
#define MAX_NAME_LEN_TUNABLE_PARS   30
#define REQMSG_GET_NAMES            0
#define REQMSG_GET_DEFAULTS         1
#define REQMSG_SET_PARS             2
#define REQMSG_SET_PAR_BY_ID		3 /* overwrite single parameter (ID:value) */
#define REQMSG_RESET_PAR_BY_ID		4 /* reset single parameter to initial value (ID) */
#define REQMSG_ADJUST_PAR_BY_ID		5 /* adjust single parameter by multiplication with factor (ID:factor) */

/* port data of tuning servers*/
#define PORT_STRATEGY_TUNING        9017
#define PORT_MOTION_TUNING          9018
#define PORT_WORLDMODEL_TUNING      9019
#define PORT_VISION_TUNING          9020

/*---field dependent parameters begin---*/
#define DEFAULTFIELDSELECTION 'TUE_field'
#define CORNERRADIUS 0.595000
#define CIRCLERADIUS 1.045000
#define FIELDWIDTH 8.100000
#define FIELDLENGTH 12.090000
#define PENALTYAREAWIDTH 5.100000
#define PENALTYAREALENGTH 1.550000
#define GOALAREAWIDTH 3.100000
#define GOALAREALENGTH 0.550000
#define PENALTYSPOT 2.055000
#define GOALWIDTH 2.000000
#define GOALDEPTH 0.59000
#define WHITEPOINTS_ALLOWEDERROR 0.10000
#define FIELDMAP_SHARPNESS 10.000000
/*---field dependent parameters end---*/

/*---goal dependent parameters begin---*/
#define DEFAULTGOALSELECTION 'Wooden_TUe_goal'
#define LASERGOALWIDTH 2.005000
#define LASERGOALDEPTH 0.580000
#define LASERGOALPOSTS 0.000000
/*---goal dependent parameters end---*/
#define LASERGOALMINCONF 0.70

#define LINEWIDTH		0.125
#define LARGEGOALWIDTH 3.0
#define LARGEGOALHEIGHT 2.0

/*camera dependent settings*/
#define NOMINAL_CAMERA_MAPPING_PARAMETER1 1.67 /*old:1.7, new 1.67*/
#define NOMINAL_CAMERA_MAPPING_PARAMETER2 .0051 /*old:.007, new: .0051*/
#define SIZEX 584 /*pixels, old: 640, new: 584*/
#define SIZEY 582 /*pixels, old: 480, new: 582*/
#define NPIXELS (SIZEX*SIZEY)

#define DEFAULTSIMFIELD 'Nagoya_2017'

/* fieldline distance lookup table parameters */
#define FIELDMAP_RESOLUTION         100.0 //[1/m]
#define FIELDMAP_BORDERWIDTH        1.0 //[m]
enum { FIELDMAP_X = ((int)((FIELDWIDTH*.5+FIELDMAP_BORDERWIDTH)*FIELDMAP_RESOLUTION+.5)*2+1),
 FIELDMAP_Y = ((int)((FIELDLENGTH*.5+FIELDMAP_BORDERWIDTH)*FIELDMAP_RESOLUTION+.5)*2+1)};
#define FIELDMAP_XY (FIELDMAP_X*FIELDMAP_Y)


/* fieldline margins for (sub)target clipping */
#define BORDERMARGIN_SMALL 0.4        /* standard margin of subtarget wrt fieldlines */
#define BORDERMARGIN_LARGE 0.6        /* margin of subtarget wrt fieldlines for active refbox tasks */
#define BORDERMARGIN_LARGE_TIME 5     /* [s] hysteresis time to keep margin_large */
#define BORDERMARGIN_WORLDMODEL 0.7	  /* margin to prevent defending obstacles outside the field (such as the referee)*/

/* object parameters */
#define BALLRADIUS                  0.11
#define BALLOBSTACLERADIUS          0.4
#define OPPONENTRADIUS              0.4
#define ROBOTRADIUS                 0.33
#define SELFRADIUS                  0.28
#define BALLROBOTDIST               0.27
#define ROBOTHEIGHT                 0.7
#define NGOALSCAN 					100 			//amount of points to scan goal for clearance of obstacles
#define NGOALSCAN_FOR_GREENFIELD    25

/* green intensity reference for adaptive shutter */
#define GREENREF 70
#define DESIRED_GREEN_INTENSITY GREENREF

/* ball detection */
#define MEDIAN_FILTER_THRESHOLD     3 // [pixels] 3x3 filter, 5 for true median filter, threshold can be adjusted

/* ball velocity estimator */
#define CLIP_LOWER_BALL_VELOCITY    0.25
#define CLIP_UPPER_BALL_VELOCITY    12.0

 /* line detection */
#define MAXLINEPOINTS 100
#define NANGLES 200 

/* position & angle precision */
#define PRECISION_POSITION          0.05	/* maximum precision, mainly defined by steady state position error */
#define PRECISION_ROTATION          0.02	/* maximum precision, mainly defined by steady state position error */
#define MAX_MOVE_VELOCITY           4.0
#define MAX_MOVE_ACCELERATION       4.5
/* set target_xyo[2] to LOOK_AT_BALL_FLAG and phimaker.c will make sure
 * that the robot looks at the ball during move towards target
 * (value is set to an unexpected value of target_xyo[2] */
#define LOOK_AT_BALL_FLAG 			123456789

#define MAGENTA                     0
#define CYAN                        1

/* model samplerates */
#define MOTION_SAMPLE_RATE          1000
#define VISION_SAMPLE_RATE          50 // (1./0.016667) //60 Hz, 1/Ts is needed for round off problems
#define WORLDMODEL_SAMPLE_RATE      10
#define MULIB_SAMPLE_RATE           20

#define RATE_MOTION                 MOTION_SAMPLE_RATE
#define RATE_VISION                 VISION_SAMPLE_RATE 
#define RATE_STRATEGY               1000
#define RATE_WORLDMODEL             WORLDMODEL_SAMPLE_RATE

/* maximum delay time buffering in matching pose vision and encoders in [s] */
#define MAX_DELAY_BUFFER            0.3
#define KINECT_DELAY                (0.075*MOTION_SAMPLE_RATE) /* Delay in samples for Kinect measurements */
/* multicast communication
 * - maximum age time in milliseconds (ms) of the last message of the agent; if the
 *   last communication is older than this age, the agent is considered no longer alive.
 */
#define MCCOMM_TIMEOUT_TIME             4000
/* using ball information from peers that is older than 1 second does not make sense */
#define MCCOMM_TIMEOUT_TIME_BALL		1000
/* MCCOMM_TIMEOUT_TIME_JOYSTICK, if the age of the last update of the command 
 * is larger than this timeout, the turtle will brake for 1 second and disable joystick mode */
#define MCCOMM_TIMEOUT_TIME_JOYSTICK    1000 
#define MCCOMM_TIMEOUT_TIME_APPMAN      5000
#define TIMEOUT_APPMAN_MOTION           100
#define TIMEOUT_APPMAN_VISION           300
#define TIMEOUT_APPMAN_WORLDMODEL       1000

/*capture times
 * - determinesubtarget.c
 */
#undef CT_ENABLE /* Change to #undef to remove function */
#define CT_BUFFERLENGTH 10000
#define CT_TIMESPERSAMPLE 2

/* battery voltage monitoring */
#define MAX_BAT_VOLT                24.0
#define MIN_BAT_VOLT                20.0

/* path planning */
#define DRIBBLE_ANGLE_OFFSET            0.2     /*[rad] additional offset in subtarget angle wrt objects*/
#define SCRUM_SIDE_MOVEMENT             0.5     /*[m] subtarget movement during scrum if driving backward is no option*/
#define SUBTARGET_UPDATE_DELAY_DRIBBLE  33      /*[ms] subtarget update delay during dribble*/

/* Trajectoryplanner parameters*/
// #define EPSILON_POSITION_CONVERGED 0.005 /* sample_time*v_max+0.5*a_max*sample_time*sample_time */
// #define EPSILON_VELOCITY_CONVERGED 0.001*2.5 /* sample_time*a_max */
#define MOTION_SAMPLE_TIME 0.001
#define EPS 1e-15 /* Machine precision*/

/* general constants */
#define PI                              3.14159265358979    /*pi value*/

/* BWA possibility check */
#define MINOBSTDISTBWA              1.0                 /* distance to check obstacles behind the robot to enable / disable BWA */
#define CHECKWIDTHBWA               0.2                 /* check on line ball-robot with a certain WIDTH */
#define CHECKANGLEBWA               (10.0*M_PI/180.0)   /* check on line ball-robot with a V-shape */

/* Pass global Parameters*/
#define MAXIMUM_PASS_DISTANCE   			15.0
#define MINIMUM_PASS_DISTANCE   			3.0
#define MINIMUM_DESIRED_PASS_DISTANCE		dmax(3.0,0.33*FIELDWIDTH)
#define MINIMUM_PASS_DISTANCE_HYSTERISIS   	0.1
#define MAX_REC2DIP_DISTANCE_VELCOMPONENT   4.0

#endif
