/* Bus Service Problem
 * Mike Xu
 * EECS 338
 * 3-7-2014
 */

/* customer.c
 * - waits for ticket agent
 * - recieves ticket
 * - boards current bus or waits for next bus
 */

#include "main.h"

int main(int argc, char *argv[]) {
	struct busTicket myTicket;

	pname = argc > 0 ? argv[1] : "CUSTOMER";
	pid = getpid();
	printInfo("Initializing Customer");
	/* initialize shared resource variables */
	locateResources(&semid, &shmid, &shm);

	/* get in line */
	semWait(semid, SEM_LINE);
	printInfo("My turn in line");

	/* wait for mutex */
	semWait(semid, SEM_MUTEX);
	
	/* tell agent my name */
	snprintf(shm->name, sizeof(shm->name), "%s", pname);
	semSignal(semid, SEM_AGENT);

	/* wait for my ticket to be assigned */
	semWait(semid, SEM_TICKET);
	
	/* get the ticket */
	myTicket = shm->ticket;
	sprintf(buf, "My Ticket: [Name:%s Seat:%d Depart:%ld]",
		myTicket.name, myTicket.seat, myTicket.depart);
	printInfo(buf);

	/* release mutex */
	if(myTicket.depart == shm->nbDepart) {
		printInfo("Waiting for next bus");
		shm->nbWait++;
		semSignal(semid, SEM_MUTEX);
		/* wait for next bus */
		semWait(semid, SEM_NBUS);
	} else {
		semSignal(semid, SEM_MUTEX);
	}

	/* board the bus */
	printInfo("Boarding the bus");
	return EXIT_SUCCESS;
}
