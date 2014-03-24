/* Bus Service Problem
 * Mike Xu
 * EECS 338
 * 3-7-2014
 */

/* main.c
 * - allocates and initializes shared memory variables
 * - invokes ticket agent process
 * - invokes bus processes
 * - invokes customer processes
 * - destroys shared memory variables
 */

#include <sys/wait.h>
#include "main.h"

#define DEFAULT_BUSSES 3

/* function headers ***********************************************************/
 void allocateResources(int *semid, int *shmid, struct common **shm);

/* implementation *************************************************************/

int main(int argc, char *argv[]) {
    int             sleepTime,      /* timer for invoking bus processes */
                    randTime,       /* timer for invoking customer processes */
                    cbCustomers,    /* arriving customers for current bus */
                    nbCustomers,    /* arriving customers for next bus */
                    numCustomers,   /* total number of customers invoked */
                    numBusses,      /* total number of busses invoked */
                    maxBusses;      /* max number of busses to invoke */
    pid_t           agent;          /* pid of agent process */
    union semun     arg;            /* data structure for semaphore ops */
    ushort          sem[SEM_SIZE];  /* semaphore */
    char            busStr[32],     /* string for bus process names */
                    custStr[32];    /* string for customer process names */

    cbCustomers     = 0;
    nbCustomers     = 0;
    numCustomers    = 0;
    numBusses       = 0;
    maxBusses       = argc > 0 ? atoi(argv[1]) : DEFAULT_BUSSES;
    pname = "MAIN";
    pid = getpid();
    printHeader();
    printInfo("Initializing bus service");
    /* initialize shared resource variables */
    allocateResources(&semid, &shmid, &shm);

    /* initialize semaphore values */
    sem[SEM_MUTEX]  = 1;
    sem[SEM_LINE]   = 1;
    sem[SEM_AGENT]  = 0;
    sem[SEM_TICKET] = 0;
    sem[SEM_NBUS]   = 0;
    arg.array       = sem;
    if(semctl(semid, 0, SETALL, arg) < 0) {
        perror("semctl(SETALL)");
        exit(1);
    }

    /* initialize shared memory values */
    shm->cbSeats    = 7;
    shm->nbSeats    = 7;
    shm->nbWait     = 0;
    shm->cbDepart   = 0;
    shm->nbDepart   = BUS_INTERVAL;

    /* invoke agent process */
    printInfo("Invoking agent");
    agent = fork();
    switch(agent) {
        case 0:
            execl("agent", "agent", (char *) NULL);
            exit(0);
        case -1:
            perror("fork");
            exit(4);
    }

    while(numBusses < maxBusses) {
        numBusses++;
        printInfo("Invoking bus");
        /* invoke a bus process */
        switch(fork()) {
            case 0:
                sprintf(busStr, "BUS %d/%d", numBusses, maxBusses);
                execl("bus", "bus", busStr, (char *) NULL);
                exit(0);
            case -1:
                perror("fork");
                exit(4);
        }
        /* time until next bus */
        sleepTime = BUS_INTERVAL;

        /* reset current and next bus customer counts */
        cbCustomers = nbCustomers;
        nbCustomers = 0;
        while(nbCustomers < 7 && sleepTime > CUST_INTERVAL) {
            numCustomers++;
            if(cbCustomers < 7)
                cbCustomers++;
            else
                nbCustomers++;

            /* sleep up to CUST_INTERVAL seconds */
            randTime = rand() % CUST_INTERVAL;
            sleepTime -= randTime;
            sleep(randTime);

            /* invoke a customer process */
            printInfo("Invoking customer");
            switch(fork()) {
                case 0:
                    sprintf(custStr, "CUST %d", numCustomers);
                    execl("customer", "customer", custStr, (char *) NULL);
                    exit(0);
                case -1:
                    perror("fork");
                    exit(4);
            }
        }
        /* sleep until next bus comes */
        sleep(sleepTime);
    }
    
    /* terminate the agent */
    printInfo("Terminating agent");
    kill(agent, SIGTERM);
    wait(0);

    /* release shared resources */
    printInfo("Releasing shared resources");
    if(shmdt((void *) shm) < 0) { perror("shmdt"); }
    if(shmctl(shmid, IPC_RMID, 0) < 0) { perror("shmctl"); }
    if(semctl(semid, 0, IPC_RMID) < 0) { perror("semctl"); }
    printInfo("Terminating bus service");
    return EXIT_SUCCESS;
}

void allocateResources(int *semid, int *shmid, struct common **shm) {
    key_t key;
    /* generate unique key from filepath */
    if((key = ftok(".", 0)) == (key_t) -1) {
        perror("ftok");
        exit(1);
    }
    /* get semaphore block */
    *semid = semget(key, SEM_SIZE, IPC_CREAT | IPC_EXCL | 0666);
    if(*semid < 0) {
        perror("semget");
        exit(1);
    }
    /* get shared memory block for common */
    *shmid = shmget(key, sizeof(struct common), 
                    IPC_CREAT | IPC_EXCL | 0666);
    if(*shmid < 0) {
        perror("shmget");
        exit(1);
    }
    /* attach pointer to shared memory block */
    *shm = (struct common *) shmat(*shmid, NULL, 0);
    if(*shm == (struct common *) -1) {
        perror("shmat");
        exit(1);
    }
}
