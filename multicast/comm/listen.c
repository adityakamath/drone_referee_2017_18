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

#include "comm.h"


int end = 0;
int timer = 0;

int MAX_DELTA;

struct timeval lastSendTimeStamp;
int delay = 0;

/* debugging purposes */
#ifdef DEBUG
	FILE * fdebug;
#endif

int lostPackets[MAX_AGENTS];
int receivedPackets[MAX_AGENTS];

int packetLossDistribution[MAX_AGENTS][MAX_LOST_PACKETS_LOGGED];

#define dmin(a,b)   ((a)<=(b) ? (a):(b))
#define dabs(a)     ((a)>= 0 ? (a):-(a))

struct _agent agent[MAX_AGENTS];

int RUNNING_AGENTS = 0;

int myNumber;

// Declaration of static functions
static int parseArguments(int argc, char **argv, optionStruct_t* options);

//	*************************
//  Signal catch
//
static void signal_catch(int sig)
{
	if (sig == SIGINT || sig == SIGTERM)
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
	char* read_ptr;
	int agentNumber;
	int i;
	_frameHeader_t* frameHeader_ptr;
	int nlost = 0;
	_record_t* record_ptr=NULL;

	for (i=0; i<MAX_AGENTS; i++)
	{
	    agent[i].firstFrame = 0;
		agent[i].lastFrameCounter = 0;
		agent[i].state = NOT_RUNNING;
		agent[i].stateCounter = 0;
	}

	while(!end)
	{
		bzero(recvBuffer, BUFFER_SIZE);

		if((recvLen = receiveData((multiSocket_t*)arg, recvBuffer, BUFFER_SIZE)) > 0 )
		{
			read_ptr = recvBuffer;
			/* set frame header pointer to read pointer */
			frameHeader_ptr = (_frameHeader_t*)read_ptr;

			/* set read_ptr after frame header */
			read_ptr += sizeof(_frameHeader_t);

			agentNumber = frameHeader_ptr->number;

			if (agentNumber == myNumber)
			{
				continue;
			}
			if (agentNumber < 0 || agentNumber > MAX_AGENTS)
			{
				PERR("Incorrect agentNumber (%d) in received data",agentNumber);
				continue;
			}
			/* init */
			if (agent[agentNumber].firstFrame == 0)
			{
			    agent[agentNumber].firstFrame = frameHeader_ptr->counter;
			}
			/* init (note that this also happens after a counter overflow at sending agent) */
			if ( agent[agentNumber].lastFrameCounter == 0)
			{
			    agent[agentNumber].lastFrameCounter = frameHeader_ptr->counter - 1;
			}
			/* overflow, or incorrect order */
			if(frameHeader_ptr->counter < agent[agentNumber].lastFrameCounter)
			{
				agent[agentNumber].lastFrameCounter = frameHeader_ptr->counter - 1;
			}

			if ((agent[agentNumber].lastFrameCounter + 1) != frameHeader_ptr->counter)
			{
				nlost = frameHeader_ptr->counter - agent[agentNumber].lastFrameCounter - 1;
				lostPackets[agentNumber] += nlost;
				packetLossDistribution[agentNumber][dmin(dabs(nlost),(MAX_LOST_PACKETS_LOGGED-1))]++;
			}
			else
			{
				/* no packet lost */
				receivedPackets[agentNumber]++;
				packetLossDistribution[agentNumber][0]++;
			}
			gettimeofday(&(agent[agentNumber].receiveTimeStamp), NULL);
			agent[agentNumber].received = YES;
			agent[agentNumber].lastFrameCounter = frameHeader_ptr->counter;
			/* loop over all records and copy data into database */
			for(i = 0; i < frameHeader_ptr->noRecs; i++)
			{
				// set record pointer to correct address in receive buffer
				record_ptr = (_record_t*)read_ptr;

				// data
				if( DB_comm_put (agentNumber, record_ptr->rec_id, record_ptr->rec_size, (void*)&record_ptr->pdata, (int)record_ptr->age) < 0 )
				{
					PERR("Error in frame: agent: %d, rec id = %d", agentNumber, record_ptr->rec_id);
					break;
				}
				// set indexbuffer to next record
				read_ptr += record_ptr->rec_size + sizeof(_record_t) - sizeof(record_ptr->pdata);
			}
		}
	}

	return NULL;
}

//	*************************
//  Main
//
int main(int argc, char *argv[])
{
	multiSocket_t sckt;
	int i,j;
	pthread_t recvThread;
	FILE * fstats;
	struct sched_param proc_sched;
	pthread_attr_t thread_attr;

	optionStruct_t options;
	network_config_t network_config;
	RTDBconf_agents_comm rtdb_conf_comm;

	/* parse arguments */
	if(parseArguments(argc, argv, &options) != 0)
	{
		PERRNO("Incorrect arguments are provided");
		return -1;
	}

	/* Assign a real-time priority to process */
	proc_sched.sched_priority=60;
	if ((sched_setscheduler(getpid(), SCHED_FIFO, &proc_sched)) < 0)
	{
		PERRNO("setscheduler");
		return -1;
	}

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

	if(signal(SIGTERM, signal_catch) == SIG_ERR)
	{
		PERRNO("signal");
		return -1;
	}

	if( get_network_config( &options.network_name, &network_config ) != 0 )
	{
		PERR("Error retrieving network configuration");
		return -1;
	}

	/* set the network interface name */
	set_network_iface( &network_config, options.iface );

	if( openSocket(&sckt, &network_config) == -1 )
	{
		PERR("openMulticastSocket");
		return -1;
	}

	if(DB_init() == -1)
	{
		PERR("DB_init");
		closeSocket(&sckt);
		return -1;
	}

	myNumber = Whoami();

	if(DB_comm_ini(&rtdb_conf_comm, options.network_name) < 0)
	{
		PERR("DB_comm_ini");
		DB_free();
		closeSocket(&sckt);
		return -1;
	}

#ifdef DEBUG
	if ((fdebug = fopen("log.txt", "w")) == NULL)
	{
		PERRNO("fopen");
		DB_free();
		closeSocket(&sckt);
		return -1;
	}
#endif


	pthread_attr_init (&thread_attr);
	pthread_attr_setinheritsched (&thread_attr, PTHREAD_INHERIT_SCHED);

	/* reset statistics variables */
	for(i=0; i<MAX_AGENTS;i++){
		lostPackets[i]=0;
		receivedPackets[i]=0;
		for(j=0;j<MAX_LOST_PACKETS_LOGGED;j++){
			packetLossDistribution[i][j] = 0;
		}
	}

	if ((pthread_create(&recvThread, &thread_attr, receiveDataThread, (void *)&sckt)) != 0)
	{
		PERRNO("pthread_create");
		DB_free();
		closeSocket(&sckt);
		return -1;
	}

	printf("Agent %d started listening...\n", myNumber);

	while (!end)
	{
		pause();
	}

	printf("\nLost Packets:\n");
	for (i=0; i<7; i++)
	{
		printf("%d:%d/%d(%.0f%%)\t", i, lostPackets[i],lostPackets[i]+receivedPackets[i] ,(lostPackets[i]/((float)(0.001+lostPackets[i]+receivedPackets[i]))*100.0));
	}
	printf("\n");

	/* print statistics in file */
	if ((fstats = fopen("comm_stats.txt", "w")) != NULL)
	{
		fprintf (fstats, "Lost Packets:\n");
		for (i=0; i<7; i++)
		{
			fprintf (fstats,"%d:%d/%d(%.0f%%)\t", i, lostPackets[i],lostPackets[i]+receivedPackets[i] ,(lostPackets[i]/((float)(0.001+lostPackets[i]+receivedPackets[i]))*100.0));
		}
		fprintf (fstats, "\nLoss Distribution:");
		for(j=0;j<MAX_LOST_PACKETS_LOGGED;j++)
		{
			fprintf (fstats, "\n");
			for (i=0; i<7; i++)
			{
				fprintf (fstats, "%d\t", packetLossDistribution[i][j]);
			}
		}
		fprintf(fstats,"\n");
		fclose(fstats);
	}

	printf("listening	: STOPPED.\nCleaning process...\n");

#ifdef DEBUG
	fclose (fdebug);
#endif

	closeSocket(&sckt);

	pthread_join(recvThread, NULL);

	DB_free();

	printf("listening: FINISHED.\n");

	return 0;
}


static void printHelp(void)
{
	printf("\nUsage : comm <network iface> -network <network-name> [OPTIONS]\n\n");
	printf("Mandatory arguments:\n");
	printf("  <network iface>         : Name of the network interface to be used by comm\n");
	printf("                            (Should be first argument)\n\n");
	printf("Optional arguments:\n");
	printf("  -network <network-name> : Name of the network, used to find correct network\n");
	printf("                            settings as specified in config/network_conf.\n");
	printf("							If not network is provided, the network which is \n");
	printf("							marked 'default' in config/network_conf will be used.\n");
	printf(" 							If the network is defined in rtdb.conf as well,\n");
	printf("                            then only the specified shared records are sent.\n");
}

/** parseArguments()
 * Parse the arguments of the listen, returns a structure with the options
 */
static int parseArguments(int argc, char **argv, optionStruct_t* options)
{
	int i;

	/* reset the options struct */
	memset(options, 0, sizeof(optionStruct_t) );

	if(argc < 2){
		printHelp();
		return -1;
	}

	/* set network interface */
	options->iface = argv[1];

	/* loop over other arguments and parse options. Loop start at two two skip first two arguments
	 * (process name and network interface). */
	for(i=2;i<argc;i++)
	{
		if ( strncmp("-network", argv[i], sizeof("-network")) == 0 )
		{
			if( i+1 < argc )
			{
				options->network_name = argv[i+1];
			}
		}
	}

	return 0;
}

