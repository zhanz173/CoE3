#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>

struct sembuf wait_use[2] = {{0,0,0},{0,1,0}}; // waiting and incrementing
struct sembuf post[1] = {{0,-1,0}}; // decrement

// semctl() calls
union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                (Linux-specific) */
} sem_attr,ZEROS;

key_t key = ftok("sem.h",'s');