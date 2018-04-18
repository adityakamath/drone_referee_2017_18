#ifndef CONSTANT_PARAMETERS
#define CONSTANT_PARAMETERS

#include "global_par.h"

/*parameters which are required to be constant*/
#define NBALLS 50 /*amount of candidate balls*/

/*constants
 * NOTE: in vision_turtle/path_planner also two constants related to field size */
#define MIRRORHEIGHT 0.72

/*parameters to determine the estimated ballsize in findball.c*/
#define BALLSIZE(m) ((-0.05970998385859*m+1.17201657991133)*m-8.39243848686579)*m+25.68131868131875;

/*findpose.c*/
#define MINLINEPOINTS (MAXLINEPOINTS/3) /*minimum amount of whitepoints that should be detected*/
#define NP 7 /*amount of optimization parameters*/
#define MP (NP+1) /*amount of simplex vertices*/

/*compasscheck.c*/
#define COMPASSANGLETHRESHOLD 60

#endif
