/* Bus Service Problem
 * Mike Xu
 * EECS 338
 * 3-7-2014
 */

/* agent.c
 * - waits for customer processes
 * - assigns tickets to customers based on remaining seats
 */

#include "main.h"

int main(int argc, char *argv[]) {
	pname = "AGENT";
	pid = getpid();
	printInfo("Initializing Agent");
	/* initialize shared resource variables */
	locateResources(&semid, &shmid, &shm);

	/* run until terminated by main */
	while(1) {
		/* wait for next customer */
		semWait(semid, SEM_AGENT);

		/* write ticket name */
		snprintf(shm->ticket.name, sizeof(shm->ticket.name), "%s", shm->name);
		sprintf(buf, "Writing ticket for %s", shm->name);
		printInfo(buf);
		
		/* set ticket seat and departure time... */
		if(shm->cbSeats > 0) {
			/* ...for current bus */
			shm->ticket.depart = shm->cbDepart;
			shm->ticket.seat = 8 - shm->cbSeats;
			shm->cbSeats--;
		} else {
			/* ...for next bus */
			shm->ticket.depart = shm->nbDepart;
			shm->ticket.seat = 8 - shm->nbSeats;
			shm->nbSeats--;
		}

		printInfo("Ticket ready");
		/* let customer know their ticket is ready */
		semSignal(semid, SEM_TICKET);
		/* release next customer in line */
		semSignal(semid, SEM_LINE);
	}
	return EXIT_SUCCESS;
}
