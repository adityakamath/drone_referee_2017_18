// gcc example.c -lm -o example -lrtdb -I../worldmodel
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>
#include<string.h>
#include<stdbool.h>

/* include UDP dependencies */
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>

/* include RTDB dependencies */
#include "rtdb/rtdb_user.h"
#include "rtdb/rtdb_api.h"

/* include record definitions */
#include "MotionShared.h"
#include "BallShared.h"

/* define config parameters */
#define total_players 8 //total number of players
#define dist_threshold 0.5 //distance threshold in meters
#define relvel_threshold 5 //threshold for relative velocity (in the direction of impact) in meters/second
#define debug 1 //reference agent for debugging

int udp_port = 1500;
int player_count = 0;
int player_id[total_players];

int end=0;
static void signal_catch(int sig)
{
    end = 1;
}

/* function to calculate distance between two agents a & b | Sets flag according to threshold*/
bool dist_flag(int a, int b, double c[total_players][5])
{
	double xdiff = c[a][0]; //- c[b][1];
	//printf("[%d], [%d], x: [%lf] \n", a, b, xdiff);
	double ydiff = c[a][1] - 4; //c[b][0];
	//printf("[%d], [%d], y: [%lf] \n", a, b, ydiff);
	double dist = sqrt(pow(xdiff, 2) + pow(ydiff, 2));
	//printf(" Distance = [%lf] \n", dist);
	bool flag_dist;
	if(dist < dist_threshold){
		flag_dist = 1;
	} else {flag_dist=0;}
	return flag_dist;
}

/* function to calculate relative velocity between two agents a & b (along the line between the agents) | sets flag according to threshold*/
bool relvel_flag(int a, int b, double c [total_players][5]){
	//WRITE CODE HERE;
	float relvel = 4;	
	int flag_relvel;
	if(abs(relvel) < 5 && abs(relvel) > 0){
		if(relvel < 0){
			flag_relvel = 2;
		}
		if(relvel > 0){
			flag_relvel = 1;
		}
	} else {flag_relvel = 0;}
	return flag_relvel;
}

int main(int argc, char *argv[])
{
    if(argc != 2){
	printf("Usage: %s <remote IP>\n", argv[0]);
	exit(0);
    }

    /* check if the AGENT environment variable is set. 1-9 for a turtle, 11 to 30 for a devpc/coach */
    if (getenv("AGENT") == NULL){
        printf("Error: set AGENT env variable to a valid number  1-9 for a turtle, 11 to 30 for a devpc/coach.\n");
        return -1;
    }

    /* initialize the RTDB instance locally */
    int ret=0;
    ret = DB_init();
    if (ret != 0) {
        printf("Something went wrong with DB_init. Bailing out.\n");
        return 0;
    }

    char *remote_ip = argv[1];
    int sockfd;
    struct sockaddr_in serverAddr;
    char buffer[1024];
    socklen_t addr_size;

    sockfd=socket(PF_INET, SOCK_DGRAM, 0);
    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family=AF_INET;
    serverAddr.sin_port=htons(udp_port);
    serverAddr.sin_addr.s_addr=inet_addr(remote_ip);

    struct collisionFlag{
	double detection;
	double aggressor;
	double victim;
    } flag;

    int age_MS, age_BS;
    int turtle_id;
    int timeout_turtle_alive_milliseconds = 1000;
    struct MotionShared MS;
    struct BallShared BS;

    double player_state[total_players][5];
    double collision_matrix[total_players][total_players];
    double relvel_matrix[total_players][total_players];

    signal(SIGTERM, signal_catch);
    signal(SIGINT, signal_catch);

    for(turtle_id=1;turtle_id<total_players+1;turtle_id++){
			/* read the record BALL_SHARED from rtdb of turtle with id turtle_id. age_BS contains the age of the data */
		age_BS = DB_get(turtle_id, BALL_SHARED, &BS);
		age_MS = DB_get(turtle_id, MOTION_SHARED, &MS);
		if(age_MS < timeout_turtle_alive_milliseconds){
			player_id[player_count] = turtle_id;
			player_count +=1;
			printf("Turtle[%d] | Player[%d]\n",turtle_id, player_count);
		}
		else{
			printf("Turtle[%d] is not alive\n",turtle_id);
		}

	}

	while(!end){

		/* stores position and velocity data of live agents */		
		for(int playerID=0; playerID<player_count; playerID++){
			/* read the record BALL_SHARED from rtdb of turtle with id turtle_id. age_BS contains the age of the data */
			age_BS = DB_get(player_id[playerID], BALL_SHARED, &BS);
			age_MS = DB_get(player_id[playerID], MOTION_SHARED, &MS);
			if(age_MS < timeout_turtle_alive_milliseconds){
				/* print the current position, heading and velocity of the turtle, translate from mm to meters */
				//printf("Turtle[%d] heading: [%d]\n", turtle_id, MS.current_xyo[2]);
				//printf("Turtle[%d] velocity : [%f.%f]\n",turtle_id,MS.current_xy_dot[0]/1000.0,MS.current_xy_dot[1]/1000.0);
				
				printf("Turtle[%d] position : [%f, %f]\n",player_id[playerID],MS.current_xyo[0]/1000.0,MS.current_xyo[1]/1000.0);
				
				/* store turtle data into player_state matrix */
				player_state[playerID][0] = MS.current_xyo[0]/1000.0; //x
				player_state[playerID][1] = MS.current_xyo[1]/1000.0; //y
				player_state[playerID][2] = MS.current_xyo[2]; //heading
				player_state[playerID][3] = MS.current_xy_dot[0]; //velocity in x
				player_state[playerID][4] = MS.current_xy_dot[1]; //velocity in y

				/*
				if(BS.seeBall){
					printf("Turtle[%d] sees the ball at position : [%f.%f]\n",turtle_id,BS.mergedBall_xyz_xyzdot[0]/1000.0,BS.mergedBall_xyz_xyzdot[1]/1000.0);
					if(BS.usedBallTurtleID != turtle_id){
						printf("\tBut I will use the ball seen by turtle%d because he is closer to the ball\n",BS.usedBallTurtleID);
					}
				}else{
					printf("Turtle[%d] does not see the ball\n",turtle_id);
					if(BS.usedBallTurtleID != turtle_id){
						printf("\tBut I will use the ball seen by turtle%d\n", BS.usedBallTurtleID);
					}
				}
				*/

			}else{
				printf("Turtle[%d] is dead\n",player_id[playerID]);
			}

		}
		
		/* Create matrices to store distances and relative velocities between agents. Example with 4 agents:
		| |1|2|3|4|
		|1|a|b|c|d|
		|2|e|f|g|h|
		|3|i|j|k|l|
		|4|m|n|o|p|
		for example: element g represents distance/velocity to Agent3 (target) from Agent2 (reference)*/
		for(int ref_agent=0;ref_agent<player_count;ref_agent++){ //for each row
			for(int target_agent=ref_agent+1;target_agent<player_count;target_agent++){ //for each column
					//calculate relative distance, velocity and aggressor agent if the ref and target were colliding
					collision_matrix[ref_agent][target_agent] = dist_flag(ref_agent, target_agent, player_state);
					if(collision_matrix[ref_agent][target_agent] == 1){
						flag.detection = 1;
						flag.aggressor = 2;
						flag.victim = 6;
						printf("[+] Collision detected\n");// | Aggressor: [2] | Victim: [6]\n");
						/*						
						int velocity_flag = relvel_flag(ref_agent, target_agent, player_state);
						switch(velocity_flag){
							case 0:
								flag.detection = 0;
								flag.aggressor = 0;
								flag.victim = 0;							
							case 1:
								flag.detection = 1;
								flag.aggressor = ref_agent;
								flag.victim = target_agent;
								break;							
							case 2:
								flag.detection = 1;
								flag.aggressor = target_agent;
								flag.victim = ref_agent;
								break;
							default:
								break;							
						}
						*/
					}
					else{
						flag.detection = 0;
						flag.aggressor = 0;
						flag.victim = 0;
						printf("[-] No collision\n"); //with Player[%d]\n", ref_agent+1);
					}
					memcpy(buffer, &flag, sizeof(flag));
					sendto(sockfd, buffer, sizeof(flag), MSG_DONTWAIT, (struct sockaddr*)&serverAddr, sizeof(serverAddr));		
			}
		} 
		
		//UDP debug segment		
		//memcpy(buffer, &flag, sizeof(flag));
		//sendto(sockfd, buffer, sizeof(flag), MSG_DONTWAIT, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
		//printf("[+]Collision detected | [%lf] collided into [%lf] \n", flag.aggressor,flag.victim);	
        	
		usleep(250000); //sleeps for 0.25 seconds
	}
	DB_free();

	return 0;
}
