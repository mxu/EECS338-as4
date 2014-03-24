#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define SEM_MUTEX       0           /* mutex to lock shared variables */
#define SEM_LINE        1           /* blocks customers in line */
#define SEM_AGENT       2           /* blocks agent until needed by customer */
#define SEM_TICKET      3           /* blocks customer until ticket is ready */
#define SEM_NBUS        4           /* blocks customers waiting for next bus */
#define SEM_SIZE        5           /* number of semaphores */
#define BUS_INTERVAL    30          /* seconds between each bus departure */
#define CUST_INTERVAL   7           /* max seconds between each customer */

/* data structures ************************************************************/
union semun {
    int                 val;        /* value for SETVAL */
    struct semid_ds     *buf;       /* buffer for IPC_STAT and IPC_SET */
    ushort              *array;     /* array for GETALL, SETALL */
    struct seminfo      *__buf;     /* Linux buffer for IPC_INFO */
};

struct busTicket {
    char                name[32];   /* customer name */
    long                depart;     /* departure time */
    ushort              seat;       /* seat number */
};

struct common {
    ushort              cbSeats;    /* seats remaining on current bus */
    ushort              nbSeats;    /* seats remaining on next bus */
    ushort              nbWait;     /* customers waiting for next bus */
    char                name[32];   /* current customer name */
    long                cbDepart;   /* departure time of current bus */
    long                nbDepart;   /* departure time of next bus */
    struct busTicket    ticket;     /* issued ticket */ 
};

/* variable declarations ******************************************************/
pid_t                   pid;        /* process id */
int                     semid,      /* semaphore id */
                        shmid;      /* shared memory id */
struct common           *shm;       /* shared memory pointer */
char                    buf[64];    /* buffer for writing strings */
char                    *pname;     /* process name */

/* shared functions ***********************************************************/
void locateResources(int *semid, int *shmid, struct common **shm) {
    key_t key;
    /* generate unique key from filepath */
    if((key = ftok(".", 0)) == (key_t) -1) {
        perror("ftok");
        exit(1);
    }
    /* get semaphore block */
    *semid = semget(key, SEM_SIZE, 0666);
    if(*semid < 0) {
        perror("semget");
        exit(1);
    }
    /* get shared memory block */
    *shmid = shmget(key, sizeof(struct common), 0666);
    if(*shmid < 0) {
        perror("shmget");
        exit(1);
    }
    /* attach pointer to shared memory block */
    *shm = (struct common *)shmat(*shmid, NULL, 0);
    if(*shm == (struct common *) -1) {
        perror("shmat");
        exit(1);
    }
}

void semWait(int semid, int sem) {
    struct sembuf buf;
    buf.sem_op = -1;
    buf.sem_flg = 0;
    buf.sem_num = sem;
    semop(semid, &buf, 1);
}

void semSignal(int semid, int sem) {
    struct sembuf buf;
    buf.sem_op = 1;
    buf.sem_flg = 0;
    buf.sem_num = sem;
    semop(semid, &buf, 1);
}

void printHeader() {
    printf(" %-10s | %-8s | %s\n", "PROCESS", "PID", "STATUS");
    printf("------------+----------+----------\n");
    fflush(stdin);
}

void printInfo(char *msg) {
    printf(" %-10s | %-8d | %s\n", pname, pid, msg);
    fflush(stdin);
}




