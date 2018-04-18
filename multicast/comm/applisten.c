
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>

#include <unistd.h>

#include <sys/time.h>
#include <sched.h>

#include <stdlib.h>

#include "multicast.h"

#include "../rtdb/rtdb_comm.h"
#include "../rtdb/rtdb_user.h"

#include "../worldmodel/MotionShared.h"
#include "../worldmodel/BallShared.h"
#include "../worldmodel/WorldmodelShared.h"

#include "app.h"
#include "udp.h"

#define BUFFER_SIZE 1000

#define DEBUG

#define PERRNO(txt) \
	printf("ERROR: (%s / %s): " txt ": %s\n", __FILE__, __FUNCTION__, strerror(errno))

#define PERR(txt, par...) \
	printf("ERROR: (%s / %s): " txt "\n", __FILE__, __FUNCTION__, ## par)

#ifdef DEBUG
#define PDEBUG(txt, par...) \
	printf("DEBUG: (%s / %s): " txt "\n", __FILE__, __FUNCTION__, ## par)
#else
#define PDEBUG(txt, par...)
#endif

#ifdef DEBUG
#define FDEBUG(file, txt, par...) \
	fprintf(file, txt , ## par)
#else
#define FDEBUG(file, txt, par...)
#endif

#define NOT_RUNNING		0
#define RUNNING			1
#define INSERT			2
#define REMOVE			3
#define MAX_INS_REM		10

#define NO	0
#define YES	1

#define  APPSERVER		"131.155.102.178"
#define  PORT			8888
int end = 0;
int timer = 0;

int readyToGo=0;
udpData id;
appData ad;

int MAX_DELTA;

struct timeval lastSendTimeStamp;
int delay = 0;

#ifdef DEBUG
	FILE * fdebug;
#endif

int lostPackets[MAX_AGENTS];

struct _record
{
	int id;			// id
	int size;		// data size
	int life;		// life time
	void* pData;	// pointer to data
};

struct _frameHeader
{
	int number;						// agent number
	unsigned int counter;			// frame counter
	char stateTable[MAX_AGENTS];	// table with my vision of each agent state
	int noRecs;						// number of records
};

struct _agent
{
	char state;							// current state
	char stateCounter;					// counter to insert and remove
	char inFramePos;					// position in frame
	char received;						// received from agent in the last Ttup?
	struct timeval receiveTimeStamp;	// last receive time stamp
	int delta;							// delta
	unsigned int lastFrameCounter;		// frame number
	char stateTable[MAX_AGENTS];		// vision of agents state
};

struct _agent agent[MAX_AGENTS];

int RUNNING_AGENTS = 0;

int in_frame = 0;


//	*************************
//  Signal catch
//
static void signal_catch(int sig)
{
	if (sig == SIGINT)
		end = 1;
	else
		if (sig == SIGALRM)
			timer = 1;
}



//	*************************
//  Receive Thread
//
//  Input:
//		int *sckt = pointer of socket descriptor
//
void *receiveDataThread(void *arg)
{
	int recvLen;
	char recvBuffer[BUFFER_SIZE];
	int indexBuffer;
	int agentNumber;
	int i;
	RTDBconf_var rec;
	int life;
	int myNumber;
	struct _frameHeader frameHeader;

	int size;

	myNumber = Whoami();

	for (i=0; i<MAX_AGENTS; i++)
	{
		agent[i].lastFrameCounter = 0;
		agent[i].state = NOT_RUNNING;
		agent[i].stateCounter = 0;
	}

	while(!end)
	{
		bzero(recvBuffer, BUFFER_SIZE);
		indexBuffer = 0;

		if((recvLen = receiveData(*(int*)arg, recvBuffer, BUFFER_SIZE)) > 0 )
		{
			memcpy (&frameHeader, recvBuffer + indexBuffer, sizeof(frameHeader));
			indexBuffer += sizeof(frameHeader);

			agentNumber = frameHeader.number;

			if (agentNumber == myNumber)
				continue;

			// TODO correction when frameCounter overflows
			if ((agent[agentNumber].lastFrameCounter + 1) != frameHeader.counter)
				(lostPackets[agentNumber])++;
			agent[agentNumber].lastFrameCounter = frameHeader.counter;

			for (i = 0; i < MAX_AGENTS; i++)
				agent[agentNumber].stateTable[i] = frameHeader.stateTable[i];

			gettimeofday(&(agent[agentNumber].receiveTimeStamp), NULL);

			for(i = 0; i < frameHeader.noRecs; i++)
			{
				// id
                if ( rec.id < 0 || rec.id > MAX_RECS) {
                    PERR("Received invalid item id %d (from agent %d)", rec.id, agentNumber);
                    break;
                }
				memcpy (&rec.id, recvBuffer + indexBuffer, sizeof(rec.id));
				indexBuffer += sizeof(rec.id);

				// size
                if ( rec.size < 0 || rec.size > BUFFER_SIZE) {
                    PERR("Received invalid item size %d for id %d (from agent %d)", rec.size, rec.id, agentNumber);
                    break;
                }
				memcpy (&rec.size, recvBuffer + indexBuffer, sizeof(rec.size));
				indexBuffer += sizeof(rec.size);

				// life
				memcpy (&life, recvBuffer + indexBuffer, sizeof(life));
				indexBuffer += sizeof(life);

				// data
				if((size = DB_comm_put (agentNumber, rec.id, rec.size, recvBuffer + indexBuffer, life)) != (int)rec.size)
				{
					PERR("Error in frame: id = %d, pretended size = %d, returned size = %d, agent = %d", rec.id, rec.size, size, agentNumber);
					break;
				}

				indexBuffer += rec.size;
			}
		}

//              data received
                readyToGo=1;

	}

	return NULL;
}



//	*************************
//  Main
//
int main(int argc, char *argv[])
{
	int sckt;
	pthread_t recvThread;
	int sharedRecs;
	RTDBconf_var rec[MAX_RECS];
	int i, ok, iret;

	struct sched_param proc_sched;
	pthread_attr_t thread_attr;

        struct MotionShared MS;
        struct BallShared BS;
        struct WorldmodelShared WS;      

        appData ad;

	if (argc < 2)
	{
		printf("Usage: listen <interface_name>\n\n");
		return (-1);
	}

	/* Assign a real-time priority to process */
	//proc_sched.sched_priority=60;
	//if ((sched_setscheduler(getpid(), SCHED_FIFO, &proc_sched)) < 0)
	//{
	//	PERRNO("setscheduler");
	//	return -1;
	//}

	if(signal(SIGALRM, signal_catch) == SIG_ERR)
	{
		PERRNO("signal");
		return -1;
	}

	if(signal(SIGINT, signal_catch) == SIG_ERR)
	{
		PERRNO("signal");
		return -1;
	}

	if((sckt = openSocket(argv[1])) == -1)
	{
		PERR("openMulticastSocket");
		printf("\nUsage: listen <interface_name>\n\n");
		return -1;
	}

	if(DB_init() == -1)
	{
		PERR("DB_init");
		closeSocket(sckt);
		return -1;
	}

	if((sharedRecs = DB_comm_ini(rec)) < 1)
	{
		PERR("DB_comm_ini");
		DB_free();
		closeSocket(sckt);
		return -1;
	}

#ifdef DEBUG
	if ((fdebug = fopen("log.txt", "w")) == NULL)
	{
		PERRNO("fopen");
		DB_free();
		closeSocket(sckt);
		return -1;
	}
#endif

	pthread_attr_init (&thread_attr);
	pthread_attr_setinheritsched (&thread_attr, PTHREAD_INHERIT_SCHED);

	for(i=0; i<MAX_AGENTS;i++)
		lostPackets[i]=0;

	if ((pthread_create(&recvThread, &thread_attr, receiveDataThread, (void *)&sckt)) != 0)
	{
		PERRNO("pthread_create");
		DB_free();
		closeSocket(sckt);
		return -1;
	}

	printf("Agent %d started listening...\n", Whoami());

	while (!end)
	{
//              this is a cheat
//              readyToGo=1;
                if ( readyToGo ) {
                        printf("Data from robots has been received.\n");
//                      there's data available, see if we can pass through data to the app server
                        if ( udp_client_open(APPSERVER, PORT, &id)<0 ) {
                                printf("Error opening udp client.\n");
                                break;
                        }
                        printf("Client initialized.\n");

                        ok=1;
                        while (ok && !end) {

//                              prepare packet
                                for (i=0; i<MAX_ACTIVE_TURTLES; i++) {
                                        iret=DB_get(i+1, MOTION_SHARED, &MS);
                                        memcpy(ad.td[i].current_xyo, MS.current_xyo, 3*sizeof(short));
                                        ad.td[i].blueIsHomeGoal=MS.blueIsHomeGoal;
                                        ad.td[i].teamColor=MS.teamColor;
                                        memcpy(ad.td[i].waypoints_xy, MS.waypoints_xy, 2*8*sizeof(short));
                                        ad.td[i].ms_age=iret;
                                        iret=DB_get(i+1, BALL_SHARED, &BS);
                                        memcpy(ad.td[i].mergedBall_xyz_xyzdot, BS.mergedBall_xyz_xyzdot, 6*sizeof(short));
                                        iret=DB_get(i+1, WORLDMODEL_SHARED, &WS);
                                        memcpy(ad.td[i].opponent_xydxdy, WS.opponent_xydxdy, 4*MAX_OPPONENTS*sizeof(short));
                                }
				ad.ingame=1;

//                              send app packet
                                if ( udp_client_send((void*) &ad, sizeof(appData), &id)<0 ) {
                                    udp_client_close(&id);
                                    udp_client_open(APPSERVER, PORT, &id);
                                    printf("Could not send packet.\n");
                                    //ok=0;
                                } else {
                                        printf(".\n");
                                }

//                              sleep for 100 ms
                                usleep(100*1000);
                                //sched_yield();
                                //sleep(1);

                        }

//                      close client
                        udp_client_close(&id);
                        printf("Client closed.\n");

                } else {
                        sleep(1);
                }
	}

	FDEBUG (fdebug, "\nLost Packets:\n");
	for (i=0; i<MAX_AGENTS; i++)
		FDEBUG (fdebug, "%d\t", lostPackets[i]);
	FDEBUG (fdebug, "\n");

	printf("communication: STOPPED.\nCleaning process...\n");

#ifdef DEBUG
	fclose (fdebug);
#endif

	closeSocket(sckt);// this forces recv to return.

	pthread_join(recvThread, NULL);

	DB_free();

	printf("communication: FINISHED.\n");

	return 0;
}
