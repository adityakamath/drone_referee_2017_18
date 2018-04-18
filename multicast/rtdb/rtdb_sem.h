
/**
 * Create a semaphore with a certain universal (inter-process) key.
 * In case the semaphore is created the first time after system boot,
 * it is initialised as well.
 * There is no _close() or _destroy() call since no process should
 * decide when to cleanup the semaphore.
 * @return the semaphore id (sem_id) or -1 on error.
 */
int rtdb_sem_open(int key);

/**
 * Locks the semaphore with a given id.
 * The lock is released when your program calls rtdb_sem_unlock(), exits or crashes.
 */
int rtdb_sem_lock(int sem_id);

/**
 * Tries to lock the semaphore with a given id.
 * Returns -1 if the lock could not be taken (other process holds the lock), 0 if it was available and was taken.
 * The lock is released when your program calls rtdb_sem_unlock(), exits or crashes.
 */
int rtdb_sem_trylock(int sem_id);

/**
 * Unlocks the semaphore with a given id.
 */
int rtdb_sem_unlock(int sem_id);

