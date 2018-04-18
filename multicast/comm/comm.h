/*
 * comm.h
 *
 *  Created on: Jul 1, 2014
 *      Author: Koen Meessen
 */

#ifndef COMM_H_
#define COMM_H_

#include "multicast.h"
#include "rtdb_comm.h"
#include "network_config.h"

#define BUFFER_SIZE 1000

/**
 * This is the assumed network delay of packets traveling over the network
 */
#define COMM_DELAY_MS 3
#define COMM_DELAY_US (COMM_DELAY_MS*1E3)
#define MIN_UPDATE_DELAY_US 1E3

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

/* maximum number of consecutive lost packets taken into account in loss
 * distribution logging is MAX_LOST_PACKETS_LOGGED
 */
#define MAX_LOST_PACKETS_LOGGED 50

/* Synchronized TDMA seems to result in more packet loss than sending packets
 * at random times. (found by CAMBADA) Therefore, the SYNCED_TDMA is not used
 * anymore (05 July 2014)
 */

//#define SYNCED_TDMA

#ifdef SYNCED_TDMA
#pragma pack(push)
#pragma pack(1)
typedef struct _frameHeader_tag
{
	unsigned char number;			// agent number
	unsigned short counter;			// frame counter
	char stateTable[MAX_AGENTS];	// table with my vision of each agent state
	unsigned char  noRecs;			// number of records
}_frameHeader_t;
#pragma pack(pop)
#else
#pragma pack(push)
#pragma pack(1)
typedef struct _frameHeader_tag
{
	unsigned char number;			// agent number
	unsigned short counter;			// frame counter
	unsigned char  noRecs;			// number of records
}_frameHeader_t;
#pragma pack(pop)
#endif

#pragma pack(push)
#pragma pack(1)
typedef struct _record_tag
{
	unsigned char   rec_id;			// record ID
	unsigned short  rec_size;		// record size
	unsigned short  age;			// data age
	void*			pdata;			// pointer to the data
}_record_t;
#pragma pack(pop)

struct _agent
{
	char state;							// current state
	char stateCounter;					// counter to insert and remove
	char inFramePos;					// position in frame
	char received;						// received from agent in the last Ttup?
	struct timeval receiveTimeStamp;	// last receive time stamp
	int delta;							// delta
    unsigned int firstFrame;
	unsigned int lastFrameCounter;		// frame number
	char stateTable[MAX_AGENTS];		// vision of agents state
};

typedef struct optionStruct_tag
{
	int do_fork;
	char* iface;
	char* network_name;
}optionStruct_t;

#endif /* COMM_H_ */
