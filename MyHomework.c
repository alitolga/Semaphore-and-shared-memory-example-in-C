#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/errno.h>
#include <sys/stat.h>

// decrement operation
void sem_wait(int sem_id, int val){ // P signal
    struct sembuf semaphore;
    semaphore.sem_num = 0;
    semaphore.sem_op = (-1 * val);
    semaphore.sem_flg = 1;
    semop(sem_id, &semaphore, 1);
}

// increment operation
void sem_signal(int sem_id, int val){ // V signal
    struct sembuf semaphore;
    semaphore.sem_num = 0;
    semaphore.sem_op = val;
    semaphore.sem_flg = 1;
    semop(sem_id, &semaphore, 1);
}

void mySignal(int sigNum){ 
    //printf("Received signal with num = %d.\n", sigNum); 
}

void mySigSet(int num){
    struct sigaction mySigAction;
    mySigAction.sa_handler = (void *)mySignal;
    mySigAction.sa_flags = 0;
    // sigaction() system call is used to change the action taken by
    // a process on receipt of a specific signal (specified with num)
    sigaction(num, &mySigAction, NULL);
}

#define KEYSHM 1
#define KEYSEM 2
#define KEYSEM2 3

int main(int argc, char *argv[]){

	mySigSet(12);

	// File operations
	FILE *fp;
	fp = fopen(argv[1], "r");
	if(fp == NULL) {
    	perror("Error in opening file");
    	return(-1);
	}

	// Reading until the end of file and closing the file so that
	// no unexpected behaviours occur when forking.
	int watercapacity;
	fscanf(fp, "%d", &watercapacity);
	char input[200000], c;
	int index = 0;
	while(1){
		c = fgetc(fp);
		//printf("c is: %c\n", c);
		
		if(c == EOF || c == '\0'){
			//printf("c is top: %c\n", c);
			break;
		}
		if(c == ' ' || c == '\n' || c == '\r'){
			//printf("c is bottom: %c\n", c);
			continue;
		}
		input[index] = c;
		index++;
	}
	input[index] = '\0';
	fclose(fp);
	
	// Shared Memory variables
	int shmid = 0;
	int *waterlevel = NULL;
	
	printf("%s\n", input);
	printf("%d\n", watercapacity);
	
	shmid = shmget(KEYSHM, sizeof(int), 0700|IPC_CREAT);
	waterlevel = (int *) shmat(shmid, 0, 0);
	*waterlevel = watercapacity;

	// Integer variables
	int type = 5;
	int f, x;
	int waterControllerID, controllerID;

	// Semaphore variables
	int empty = 0, full = 0;
	empty = semget(KEYSEM, 1, 0700|IPC_CREAT);
	semctl(empty, 0, SETVAL, 0);
	full = semget(KEYSEM2, 1, 0700|IPC_CREAT);
	semctl(full, 0, SETVAL, 0);

	// Simulation
	printf("SIMULATION BEGINS\n");
	index = 0;

	f = fork();
	if(f!=0){
		controllerID = f;
		waterControllerID = -1;
	}
	else{
		waterControllerID = getpid();
	}


	while(1){
		if(waterControllerID == getpid()){
			break;
		}

		c = input[index];
		//printf("c is: %c\n", c);
		if(c == '\0'){
			break;
		}

		type = (c - '0');
		printf("Current water level %d cups\n", *waterlevel);
		
		f = fork();

		if(f != 0){
			pause();
		}
		else{
			printf("Employee %d wants coffee Type %d\n", getpid(), type);
			x = *waterlevel;
			if(x-type < 1){
				//printf("Main empty signal is %d\n", semctl(empty, 0, GETVAL, 0));
				printf("Employee %d WAITS\n", getpid());
				sem_signal(empty, 1);
				sem_wait(full, 1);
			}
			
			*waterlevel -= type;
			printf("Employee %d SERVED\n", getpid());
			//printf("Child: %d\n", waterControllerID);
			kill(getppid(), 12);
			exit(0);
		}
		//sleep(1);
		index++;
	}

	while(waterControllerID == getpid()){
		//printf("Empty signal is: %d\n", semctl(empty, 0, GETVAL, 0));
		sem_wait(empty, 1);
		printf("Employee %d wakes up and fills the coffee machine\n", getpid());
		*waterlevel = watercapacity;
		printf("Current water level %d cups\n", *waterlevel);
		sem_signal(full, 1);
	}

	kill(controllerID, 9);
	shmdt(waterlevel);
	semctl(shmid, 0, IPC_RMID, 0);
	semctl(empty, 0, IPC_RMID, 0);
	semctl(full, 0, IPC_RMID, 0);

	printf("SIMULATION ENDS\n");

	return 0;
}