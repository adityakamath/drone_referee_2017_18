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

#ifdef _DCOMM
#warning "THIS DCOMM IS ONLY FOR DEBUGGING PURPOSES AND SHOULD NEVER BE INSTALLED"
#endif

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
#define CLIP_SHORT(A) ((A)>INT16_MAX?(short)INT16_MAX:((A)<INT16_MIN?(short)INT16_MIN:((short)(A))))

struct _agent agent[MAX_AGENTS];

int RUNNING_AGENTS = 0;
float TTUP_US=1E6;

int in_frame = 0;
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

int sync_TDMA(int agentNumber)
{
	int realDiff, expectedDiff;
	struct itimerval it;

	if ((agent[agentNumber].state == NOT_RUNNING) || (agent[agentNumber].state == INSERT))
	{
		PDEBUG("*****  agent %d - NOT_RUNNING or INSERT  *****", agentNumber);
		return (1);
	}

	// real difference with average medium comm delay
	realDiff = (int)((agent[agentNumber].receiveTimeStamp.tv_sec - lastSendTimeStamp.tv_sec)*1E6 + agent[agentNumber].receiveTimeStamp.tv_usec - lastSendTimeStamp.tv_usec);
	realDiff -= (int)COMM_DELAY_US; // travel time
	if (realDiff < 0)
	{
		PDEBUG("*****  realDiff to agent %d = %d  *****", agentNumber, realDiff);
		return (2);
	}

	// expected difference
	expectedDiff = (int)((agent[agentNumber].inFramePos - agent[myNumber].inFramePos) * TTUP_US / RUNNING_AGENTS);
	if (expectedDiff < 0)
		expectedDiff += (int)TTUP_US;

	agent[agentNumber].delta = realDiff - expectedDiff;

	// only dynamic agent 0 make adjustments
	if (agent[myNumber].inFramePos == 0)
	{
		if ((agent[agentNumber].delta > delay) && (agent[agentNumber].delta < MAX_DELTA))
		{
			// avoid small corrections
			if (agent[agentNumber].delta > (int)MIN_UPDATE_DELAY_US)
			{
				delay = agent[agentNumber].delta;
				PDEBUG("delay between %d(%d) and %d(%d) -> %d", myNumber, agent[myNumber].inFramePos, agentNumber, agent[agentNumber].inFramePos, delay);
			}
		}
	}
	else
	{
		// only sync from dynamic agent 0
		if (agent[agentNumber].inFramePos == 0)
		{
			expectedDiff = (int)(TTUP_US - expectedDiff);
			expectedDiff -= (int)COMM_DELAY_US; // travel time
			it.it_value.tv_usec = (long int)(expectedDiff % (int)1E6);
			it.it_value.tv_sec = (long int)(expectedDiff / (int)1E6);
			it.it_interval.tv_usec=(__suseconds_t)(TTUP_US);
			it.it_interval.tv_sec=0;
			setitimer (ITIMER_REAL, &it, NULL);
		}
	}

	return 1;
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
				if( DB_comm_put( agentNumber, record_ptr->rec_id, record_ptr->rec_size, (void*)&record_ptr->pdata, (int)record_ptr->age) < 0 )
				{
					PERR("Error in frame: agent: %d, rec id = %d", agentNumber, record_ptr->rec_id);
					break;
				}
				// set indexbuffer to next record
				read_ptr += record_ptr->rec_size + sizeof(_record_t) - sizeof(record_ptr->pdata);
			}

#ifdef SYNCED_TDMA

			for (i = 0; i < MAX_AGENTS; i++)
			{
				agent[agentNumber].stateTable[i] = frameHeader.stateTable[i];
			}

			sync_TDMA(agentNumber);
#endif

		}
	}

	return NULL;
}

void updateAgentState()
{
    /** coordinated state update of all agents */
    int i,j;
    for (i=0; i<MAX_AGENTS; i++)
	{
	    if ( i != myNumber)
		{
		    switch (agent[i].state)
			{
			case RUNNING:
			    if (agent[i].received == NO)
			    {
			    	agent[i].state = REMOVE;
			    }
			    break;
			case NOT_RUNNING:
			    if (agent[i].received == YES)
			    {
			    	agent[i].state = INSERT;
			    }
			    break;
			case INSERT:
			    if (agent[i].received == NO)
			    {
			    	agent[i].state = NOT_RUNNING;
			    }
			    else
				{
#ifdef SYNCED_RTDMA
			    	// Only set to running if all other running agents agree as well.
				    for (j = 0; j < MAX_AGENTS; j++)
				    {
						if ((agent[j].state == RUNNING) &&
							((agent[j].stateTable[i] == NOT_RUNNING) || (agent[j].stateTable[i] == REMOVE)))
						{
							break;
						}
				    }
#endif
				    agent[i].state = RUNNING;
				    PDEBUG("Agent %d detected: RUNNING   ****************",i);
				}
			    break;
			case REMOVE:
			    if (agent[i].received == YES)
			    {
			    	agent[i].state = RUNNING;
			    }
			    else
				{
				    for (j = 0; j < MAX_AGENTS; j++)
				    {
						if ((agent[j].state == RUNNING) &&
							((agent[j].stateTable[i] == RUNNING) || (agent[j].stateTable[i] == INSERT)))
						{
							break;
						}
				    }
				    // only set to not running if all other running agents agree as well.
				    agent[i].state = NOT_RUNNING;
				    PDEBUG("Removed AGENT %d: no more incomming packets.\n",i);
				}
			    break;
			}
		}
	    else
	    {
	    	agent[i].state = RUNNING;
	    }
	}


    int inFramePos = 0;
    for (i = 0; i < MAX_AGENTS; i++)
	{
	    if ((agent[i].state == RUNNING) || (agent[i].state == REMOVE))
		{
		    agent[i].inFramePos = inFramePos;
		    inFramePos ++;
		}
	    agent[myNumber].stateTable[i] = agent[i].state;
	    agent[i].received = NO;
	}

    RUNNING_AGENTS = inFramePos;
    MAX_DELTA = (int)(TTUP_US/RUNNING_AGENTS * 2/3);


}

//	*************************
//  Main
//
int main(int argc, char *argv[])
{
	multiSocket_t sckt;
	pthread_t recvThread;
	char sendBuffer[BUFFER_SIZE];
	char* write_ptr;
	int n_bytes = 0;
	int sharedRecs;
	RTDBconf_var *my_rec;
	unsigned int frameCounter = 0;
	int i,j;
	short int age;
#ifdef SYNCED_TDMA
	char delayTransmission = 0;
#endif
	int updcounter = 0;
	FILE * fstats;
	struct sched_param proc_sched;
	pthread_attr_t thread_attr;

	struct itimerval it;
	_frameHeader_t* frameHeader_ptr;
	_record_t* record_ptr;

	struct timeval tempTimeStamp;
	optionStruct_t options;
	network_config_t network_config;
	RTDBconf_agents_comm rtdb_conf_comm;

	/* parse arguments */
	if(parseArguments(argc, argv, &options) != 0)
	{
		PERR("Incorrect arguments are provided");
		return -1;
	}

#ifdef _DCOMM
	printf("WARNING: THIS COMM VERSION WILL NOT TRANSMIT DATA\n");
#endif

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

	/* set the communication frequency */
	TTUP_US = 1E6/((float)network_config.frequency);

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

	/* Retrieve my AGENT number */
	myNumber = Whoami();

	if(DB_comm_ini(&rtdb_conf_comm, options.network_name) < 0)
	{
		PERR("DB_comm_ini");
		DB_free();
		closeSocket(&sckt);
		return -1;
	}

	/* assign pointer to my record */
	my_rec = rtdb_conf_comm.shared;
	sharedRecs = rtdb_conf_comm.n_shared_recs;

	for(i = 0; i < sharedRecs; i++)
	{
		/* check */
		if ( my_rec[i].id < 0 || my_rec[i].id > MAX_RECS) {
			PERR("Not sending invalid item id %d (means corrupted shared mem)", my_rec[i].id);
			return -1;
		}
		/* check size */
		if ( my_rec[i].size < 0 || my_rec[i].size > BUFFER_SIZE) {
			PERR("Not sending invalid item size %d for id %d (means corrupted shared mem)", my_rec[i].size, my_rec[i].id);
			return -1;
		}
	}

	printf("Start sharing %d records in network : %s\n",sharedRecs, options.network_name);

#ifdef DEBUG
	if ((fdebug = fopen("log.txt", "w")) == NULL)
	{
		PERRNO("fopen");
		DB_free();
		closeSocket(&sckt);
		return -1;
	}
#endif

	if (options.do_fork == 1) {
	  int ret = fork();
	  if (ret != 0) {
	    DB_free();
	    //closeSocket(sckt);
	    return 0; // parent returns success.
	  }
	}

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

	/* Set itimer to reactivate the program */
	it.it_value.tv_usec=0;// (__suseconds_t)(8*TTUP_US);
	it.it_value.tv_sec=2;
	it.it_interval.tv_usec= (__suseconds_t)(TTUP_US);
	it.it_interval.tv_sec=0;
	setitimer (ITIMER_REAL, &it, NULL);

	printf("Agent %d started communication...\n", myNumber);

	/* seed random number generator */
	srand(time(NULL));

	while (!end)
	{

#ifdef SYNCED_TDMA
		pause();
		if (timer == 0)
			continue;

		timer = 0;

		// unsync
		// We wait until we may communicate (ie are 'in frame')
		if ((delayTransmission == 0) && (agent[myNumber].inFramePos != 0) && (in_frame == 0))
		{
			// increment the period (myNumber * 10% TTUP)
			it.it_value.tv_usec = (__suseconds_t)(0.10 * TTUP_US);
			it.it_value.tv_sec = 0;
			delayTransmission = 1;
			setitimer (ITIMER_REAL, &it, NULL);
			continue;
		}

		in_frame = 0;
		delayTransmission = 0;
		delay = 0;
#else
		double waitTime = (2*(((double) rand())/((double) RAND_MAX))-1) * TTUP_US * 0.05 + TTUP_US;
		usleep(waitTime);
#endif

		bzero(sendBuffer, BUFFER_SIZE);

		/* set write pointer to start of sendBuffer */
		write_ptr = sendBuffer;

		/* set frameHeader pointer to write_pointer */
		frameHeader_ptr = (_frameHeader_t*)write_ptr;

		// if we update the agent state every cycle, this becomes a very volatile
		// situation. So we allow 1 second for an agent to announce itself as alive.
		// In case of DCOMM, we only allow 2 cycles to have faster output of problems.
#ifndef _DCOMM
		if ( ++updcounter % network_config.frequency == 0 )
#else
		if ( ++updcounter % 2 == 0 )
#endif
		{
			updateAgentState();
		}

		// frame header
		frameHeader_ptr->number = myNumber;
		frameHeader_ptr->counter = frameCounter;
		frameCounter++;
#ifdef SYNCED_TDMA
		for (i = 0; i < MAX_AGENTS; i++)
		{
			frameHeader_ptr->stateTable[i] = agent[myNumber].stateTable[i];
		}
#endif
		frameHeader_ptr->noRecs = sharedRecs;

		/* set write pointer just after frameheader */
		write_ptr += sizeof(_frameHeader_t);

		for(i = 0; i < sharedRecs; i++)
		{
			/* set record pointer to write pointer */
			record_ptr = (_record_t*)write_ptr;
			/* fill record */
			age = CLIP_SHORT( DB_get(myNumber, my_rec[i].id, (void*)&record_ptr->pdata) );
			if(age < 0){
				age = INT16_MAX;
			}
			record_ptr->age = age;
			record_ptr->rec_id = my_rec[i].id;
			record_ptr->rec_size = my_rec[i].size;

			write_ptr += sizeof(_record_t) + my_rec[i].size - sizeof(record_ptr->pdata);
		}

		n_bytes = (write_ptr - sendBuffer);

		if (n_bytes > BUFFER_SIZE)
		{
			PERR("Pretended frame is bigger that the available buffer.");
			PERR("Please increase the buffer size in comm.c or reduce the number of disseminated records");
			break;
		}

		gettimeofday (&tempTimeStamp, NULL);
#ifndef _DCOMM		
		if (sendData(&sckt, sendBuffer, n_bytes) != n_bytes)
			PERRNO("Error sending data");
#else
#warning "THIS VERSION WILL NOT TRANSMIT DATA"
#endif
		lastSendTimeStamp.tv_sec = tempTimeStamp.tv_sec;
		lastSendTimeStamp.tv_usec = tempTimeStamp.tv_usec;
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

	printf("communication: STOPPED.\nCleaning process...\n");

#ifdef DEBUG
	fclose (fdebug);
#endif

	closeSocket(&sckt);

	pthread_join(recvThread, NULL);

	DB_free();

	printf("communication: FINISHED.\n");

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
	printf("  -fork                   : Clone the process, create an exact copy.\n\n");
}

/** parseArguments()
 * Parse the arguments of the comm, returns a structure with the options
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
		else if ( strncmp("-fork", argv[i], sizeof("-fork")) == 0 )
		{
			options->do_fork = 1;
		}
	}

	return 0;
}
