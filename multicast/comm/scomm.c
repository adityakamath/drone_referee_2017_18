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

int MAX_DELTA;

#define COMM_FREQUENCY 40 				// Hertz
#define TTUP_US (1E6/COMM_FREQUENCY)
#define PACKET_LOSS_PERCENTAGE 15

//	*************************
//  Signal catch
//
static void signal_catch(int sig)
{
	if (sig == SIGINT || sig == SIGTERM)
		end = 1;
}

int drop_packet()
{
	if(rand()/((double)RAND_MAX)*100 < PACKET_LOSS_PERCENTAGE){
		return 1;
	}else{
		return 0;
	}
}

//	*************************
//  Main
//
int main(int argc, char *argv[])
{
	int i,copy=0;
	int myNumber;
	int turtles = 10;
	int devpc = 0;

	struct sched_param proc_sched;

	printf("This program copies all turtle and devpc shared data to the current agent.\n");
	printf("Hence, this program needs to be started for each simulated turtle and your devpc, with the AGENT env set to that turtle's or devpc agent number.\n");
	printf("The first turtle starts at number 1. Assuming no more than 10 turtles.\n");

	if ( argc < 2)
	{
		printf("\nUse: scomm <devpc_num>  to specify the devpc agent number of the simulation.\n");
		return -1;
	} else {
		devpc = atoi(argv[1]);
		printf("Running simulation on devpc %d (AGENT=%d).\n\n", devpc - 10, devpc);
	}

	/* Assign a real-time priority to process */
	proc_sched.sched_priority=60;
	if ((sched_setscheduler(getpid(), SCHED_FIFO, &proc_sched)) < 0)
	{
		PERRNO("setscheduler");
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

	if(DB_init() == -1)
	{
		PERR("DB_init");
		return -1;
	}

	myNumber = Whoami();

	for(i=1; i<turtles+1;i++)
		if ( i != myNumber )
			DB_init_agent(i);
	if ( devpc != myNumber )
		DB_init_agent(devpc);

	printf("communication: STARTED...\n");

    struct timespec tv;
    clock_gettime(CLOCK_MONOTONIC, &tv);

	while (!end)
	{
		for (i = 1; i < turtles+1; i++)
			if ( i != myNumber ){
				/* create packet loss */
				if( drop_packet() ){
					continue;
				}
				DB_comm_copy_agent(i);
				usleep(1000);
			}
		if ( devpc != myNumber )
			DB_comm_copy_agent(devpc);
		copy++;

        // program next period:
        long long tn = tv.tv_nsec + TTUP_US * 1000LL;
        tv.tv_nsec = tn % 1000000000LL;
        tv.tv_sec += tn / 1000000000LL;
        // inspired by nanosleep man page for this construct:
        while ( clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &tv, NULL) != 0 && errno == EINTR ) {
            errno = 0;
        }
	}
	printf("Done %d memory copies\n",copy);

	printf("communication: STOPPED.\nCleaning process...\n");

	/** Free shared memory segment allocated by DB_init_agent() for each agent.
	 *  Since DB_free() only release the shared memory for itself(myNumber), if
	 *  we do not free the memory for the other agents, their corresponding
	 *  shared memory will be dangling.
	 */
	for (i = 1; i < turtles + 1; i++)
		if (i != myNumber)
			DB_free_agent(i);
	if (devpc != myNumber)
		DB_free_agent(devpc);

	DB_free();

	printf("communication: FINISHED.\n");

	return 0;
}
