#include "rtdb_sem.h"

#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */

/* for semid */
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

/* The calling program MUST define this union */
union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                (Linux-specific) */
};

int rtdb_sem_open(int key)
{
    int semid = -1;
    union semun semunarg;
    if ((semid = semget(key, 1, IPC_CREAT | 0777)) == -1) {
        perror("Error rtdb_sem_open::semget");
        return -1;
    }

    printf("Opened semid: %d with key %d\n", semid, key );

    semunarg.val = 0;
    struct semid_ds stats;
    stats.sem_otime = 0;
    if (semctl(semid,0,IPC_STAT,&stats) == -1) {
        perror("Error rtdb_sem_open::semctl IPC_STAT");
        return -1;
    }
    // only set the value to zero if never done so before.
    if ( (stats.sem_otime == 0) && (semctl(semid,0,SETVAL,semunarg.val) == -1)) {
        perror("Error rtdb_sem_open::semctl SETVAL");
        return -1;
    }
    return semid;
}

int rtdb_sem_lock(int semid)
{       
    struct sembuf sops[2];

    if (semid <0)
        return -1;

    // the semop set is executed atomically.
    // this is actually an atomic wait-for-free-and-lock operation,
    // much like a mutex works.
    // LOCKING IS A TWO-STEP PROCESS.

    // sem starts at zero !
    sops[0].sem_num = 0;        /* Operate on semaphore 0 */
    sops[0].sem_op = 0;         /* Wait for value to equal 0 */
    sops[0].sem_flg = 0;

    // raise it, we never lower it, so the next invocation of this function would block forever.
    sops[1].sem_num = 0;        /* Operate on semaphore 0 */
    sops[1].sem_op = 1;         /* Increment value by one */
    sops[1].sem_flg = SEM_UNDO; /* set to = 0 to have deadlock on crash or program exit */
    
    if (semop(semid, sops, 2) == -1) {
        perror("Error rtdb_sem_lock::semop");
        return -1;
    }
    return 0;
}

int rtdb_sem_trylock(int semid)
{       
    struct sembuf sops[2];

    if (semid <0)
        return -1;

    // the semop set is executed atomically.
    // this is actually an atomic wait-for-free-and-lock operation,
    // much like a mutex works.
    // LOCKING IS A TWO-STEP PROCESS.

    // sem starts at zero !
    sops[0].sem_num = 0;        /* Operate on semaphore 0 */
    sops[0].sem_op = 0;         /* Wait for value to equal 0 */
    sops[0].sem_flg = IPC_NOWAIT;

    // raise it, we never lower it, so the next invocation of this function would block forever.
    sops[1].sem_num = 0;        /* Operate on semaphore 0 */
    sops[1].sem_op = 1;         /* Increment value by one */
    sops[1].sem_flg = SEM_UNDO; /* set to = 0 to have deadlock on crash or program exit */
    
    if (semop(semid, sops, 2) == -1) {
      if (errno == EAGAIN)
	return -1;
      perror("Error rtdb_sem_trylock::semop");
      return -1;
    }
    return 0;
}



int rtdb_sem_unlock(int semid)
{
    struct sembuf sops[1];

    if (semid <0)
        return -1;

    // whichever we do (crash/return 0), the sem is locked in this point
    // the SEM_UNDO flag will release the lock in both cases.

    // unlock code:
    // UNLOCKING IS A ONE-STEP PROCESS.
    sops[0].sem_num = 0;        /* Operate on semaphore 0 */
    sops[0].sem_op = -1;        /* release sem again */
    sops[0].sem_flg = SEM_UNDO; /* it seems both =0 and SEM_UNDO have same behaviour */
    
    if (semop(semid, sops, 1) == -1) {
        perror("Error rtdb_sem_unlock::semop2");
        return -1;
    }
    return 0;
}
