#ifndef __RTDB_API_H
#define __RTDB_API_H

#include "rtdbdefs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Init the realtime database.
 * This function uses the environment variable AGENT
 * to get the agent ID.
 */
int DB_init (void);

/**
 * Init the realtime database for a given agent id.
 */
int DB_init_agent( int agent_id );

void DB_free_agent( int agent_id );

void DB_free (void);



int DB_put (int _id, void *_value);



int DB_get (int _agent, int _id, void *_value);



int Whoami(void);


int IsAlive(int _agent, int time_out);

  /**
   * Returns the usage count of the DB's shared memory.
   * You may call this function at any time, also if
   * DB_init was not done.
   * Returns -1 if it could not determine the count, 
   * Returns zero or a positive value to return the count.
   */
int DB_use_count();

  /**
   * Returns null if yes, -1 if no.
   */
int IsProcessRunning(const char* name);

    /**
     * Start comm/listen if necessary on an interface (lo, eth0 or eth1).
     * 'lo' is reserved for the simulator, where comm is broadcasting
     * on the loopback device. eth0 and eth1 are for wire/wireless respectively.
     *
     * If lo is specified, 'listen' is killed because otherwise our simulator
     * would also receive data from the real robots !
     *
     * Call this once from each process that requires comm.
     * comm will only run on one interface.
     * If comm is already running, iface is ignored and nothing is done !
     */
int RequireComm(const char* opts);

    /**
     * Check if comm/listen is running. If not, restart it on the interface.
     * Only call this after you have done RequireComm once.
     * @param iface specify the same argument as with RequireComm. You can
     * do tricks by changing the interface (eth1 -> eth0 etc.), and *then*
     * killall comm. In that case, CheckComm will restart comm on the new
     * interface.
     * If comm is already running, iface is ignored and nothing is done !
     */
int CheckComm(const char* opts);

  /**
   * Use this function to kill and restart comm on a new interface.
   * Only call this after you have done RequireComm once.
   * Be sure to change the interface name in later calls to CheckComm as well.
   */
int RestartComm(const char* opts);

    /**
     * Release requirement of comm. For each call to RequireComm(), you need to
     * call ReleaseComm() before you exit your program. If you are the last process
     * releasing comm, comm is killed.
     * This also takes into account crashing programs that did not have
     * the opportunity to call ReleaseComm. However, if the last program using comm
     * crashes, comm will not be killed.
     */
int ReleaseComm();

#ifdef __cplusplus
} 
#endif

#endif
