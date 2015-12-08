#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdio.h>

#define SYSCHK(PRED) \
  if(!(PRED)) { \
    printf("SYSCHK fail at line %d\n", __LINE__); \
    perror(#PRED); \
    exit(-1); \
}


void sekcja_krytyczna(int k) {
    char numer[10];
    int i,j,l;
    snprintf(numer, 10, "%d ", k);

    l = strlen(numer);

    for(i = 0; i < 50; i++)
        for(j = 0; j < l; j++) {
            putchar(numer[j]);
            fflush(stdout);
        }
    putchar('\n');
}

int main() {
	int shmid = shmget(IPC_PRIVATE, 100, 0700);
	SYSCHK(shmid != -1);
	void *shm = shmat(shmid, 0, 0);
	sem_t *sem = (sem_t *) shm;
	SYSCHK(sem_init(sem, IPC_PRIVATE, 1) == 0);

    int i;
    for(i = 0; i < 5; i++) {

        if (fork() != 0) { // Rodzic
        }
        else { // Potomek
            break;
        }
    }


    while(1) {
    	sem_wait(sem);
        sekcja_krytyczna(i);
        sem_post(sem);
    }
    sem_close(sem);

    return 0;
}
