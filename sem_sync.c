/*Problems to be done:
*[X] Change c file name to sem_sync
*[] Typing in sem_sync X Y Z should print the following (X Y Z are numbers)
*[X] Print “time Parent started” 
*[X] Semaphore ID = 12345678
*[X] Print “time Child X unlocking (PID=12345)”
*[X] Print “time Child Y unlocking (PID=12346)”
*[X] Print “time Child Z unlocking (PID=12347)”
*[] Print “time all obstacles removed, parent proceeds”
*[] Print “sem id (33357841) successfully removed”
*[] Check if program works with “final_check.sh” in wk14 directory
*/

/*************************************************************************\
 * *                  Copyright (C) Michael Kerrisk, 2014.                   *
 * *                                                                         *
 * * This program is free software. You may use, modify, and redistribute it *
 * * under the terms of the GNU Affero General Public License as published   *
 * * by the Free Software Foundation, either version 3 or (at your option)   *
 * * any later version. This program is distributed without any warranty.    *
 * * See the file COPYING.agpl-v3 for details.                               *
 * \*************************************************************************/

#include "curr_time.h"                      /* Declaration of currTime() */
#include "tlpi_hdr.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "semun.h"
#include <sys/sem.h>

struct sembuf sop;

int main(int argc, char *argv[])
{
    int j, dummy;
    int semid; 

    if (argc < 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s sem_sync...\n", argv[0]);

    union semun arg;

    semid = semget(IPC_PRIVATE, 1, S_IRUSR | S_IWUSR);
        if (semid == -1)
            errExit("semid");

    arg.val = 0;
        if ( semctl(semid, 0, SETVAL, arg) == -1)
            errExit("semctl");

//Add 1 to semaphore
    sop.sem_num = 0;
    sop.sem_op = 1;
    sop.sem_flg = 0;

    printf("%s  Parent started\n", currTime("%T"));

    for (j = 1; j < argc; j++) {
        switch (fork()) {
        case -1:
            errExit("fork %d", j);

        case 0: /* Child */

            /* Child does some work, and lets parent know it's done */

            sleep(getInt(argv[j], GN_NONNEG, "sleep-time"));
                                            /* Simulate processing */
            
            printf("%s  Child %d unlocking (PID=%ld)\n",
                    currTime("%T"), j, (long) getpid());
		 semop(semid, &sop, 1);
            /* Child now carries on to do other things... */

            _exit(EXIT_SUCCESS);

        default: /* Parent loops to create next child */
            break;
        }
    }
    
        printf("Semaphore ID = %d\n", semid);

    /* Parent may do other work, then synchronizes with children */

//Lock parent until children have added +1 enough times to prevent semaphore from becoming negative
    sop.sem_op = -1*(argc-1);
    semop(semid, &sop, 1);
    printf("%s  Parent ready to go\n", currTime("%T"));

    /* Parent can now carry on to do other things... */
    semctl(semid, 0, IPC_RMID, NULL);

    exit(EXIT_SUCCESS);
}



