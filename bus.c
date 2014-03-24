/* Bus Service Problem
 * Mike Xu
 * EECS 338
 * 3-7-2014
 */

/* bus.c
 * - increments departure time shared variables
 * - boards waiting customers
 * - waits until departure time
 * - departs the station
 */

#include "main.h"

int main(int argc, char *argv[]) {
	pname = argc > 0 ? argv[1] : "BUS";
	pid = getpid();
	printInfo("Initializing Bus");
	/* initialize shared resource variables */
	locateResources(&semid, &shmid, &shm);
	/* wait for mutex */
	semWait(semid, SEM_MUTEX);
	/* increment depature times */
	shm->cbDepart += BUS_INTERVAL;
	shm->nbDepart += BUS_INTERVAL;

	/* set available seat count */
	shm->cbSeats = 7 - shm->nbWait;
	shm->nbSeats = 7;

	/* board waiting customers */
	while(shm->nbWait > 0) {
		semSignal(semid, SEM_NBUS);
		shm->nbWait--;
	}

	sprintf(buf, "Departing at %ld", shm->cbDepart);
	printInfo(buf);
	sprintf(buf, "%d seats available", shm->cbSeats);
	printInfo(buf);

	/* release mutex */
	semSignal(semid, SEM_MUTEX);
	/* wait until time to depart */
	sleep(BUS_INTERVAL);
	/* depart */
	printInfo("Departing");
	return EXIT_SUCCESS;
}
