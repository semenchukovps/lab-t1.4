//#include <sys/types.h>
//#include <sys/wait.h>
//#include <sys/stat.h>
//#include <unistd.h>
//#include <string.h>
#include <stdbool.h>
//#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <semaphore.h>
#include <signal.h>


#define SEMAPHORE_NAME "/my_named_semaphore"

void * sendMsg();
void * getValue();
void hdl(int);
sem_t *sem;
int *shmem;
int shmid;
bool exec = true;

int main(int argc, char *argv[]){

	int pipefd[2];
	pid_t procA, procB, procC;
	int value, valueRead;
	
	struct sigaction act;
	memset(&act, 0, sizeof(act));
	act.sa_handler = hdl;
	
	sigemptyset(&act.sa_mask);                                                            
    sigaddset(&act.sa_mask, SIGUSR1);
    sigaction(SIGUSR1, &act, 0);
		
	if (pipe(pipefd) == -1) {
		perror("cant create pipe");
		exit(EXIT_FAILURE);
	}
		
	sem = sem_open(SEMAPHORE_NAME, O_CREAT, 0777, 0);
	sem_init(sem, 1, 0);
	shmid = shmget (IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);	
	shmem = (int *)shmat (shmid, 0, 0);

	procB = fork();
	if(procB == 0){									//Process B
		int r1, r2, result;
			pthread_t c1,c2;
			r1 = pthread_create(&c1, NULL, sendMsg, "C1");
				if (r1 != 0){
					perror("Error creating c1 thread");
					return EXIT_FAILURE;
				}
				r2 = pthread_create(&c2, NULL, getValue , NULL);
				if (r2 != 0){
					perror("Error creating c2 thread");
					return EXIT_FAILURE;
				}
				result = pthread_join(c1, NULL);
				if (result != 0){
					perror("Joining the second thread");
					return EXIT_FAILURE;
				}
			if(!exec){
				pthread_kill(c1, SIGTERM);
				pthread_kill(c2, SIGTERM);
				kill(procA, SIGTERM);
				kill(procB, SIGTERM);
			}
		}else{ 											
		procC = fork();
		if(procC != 0){
			procA = getpid();
			close(pipefd[0]);
			printf("Input integer value (10 to exit): \n");
			while(exec){
				scanf("%d", &value);
				write(pipefd[1], &value, sizeof(value));
				printf("Parent(%d) send value: %d\n", getpid(), value);
				
			}
			close(pipefd[1]);
		}else{
			procB = getpid();
			close(pipefd[1]);
			while(read(pipefd[0], &valueRead, sizeof(valueRead)) > 0){
				printf("Child(%d) received value: %d\n", getpid(), valueRead);
				valueRead *= valueRead;
				sem_post(sem);
				memcpy(shmem, &valueRead, sizeof(valueRead));
				printf("sent to shared memory: %d\n", *shmem);
			}
			close(pipefd[0]);
		}
	}
	
	sem_close(sem);
	shmdt(shmem);
    return 0;
}	

void * sendMsg(void * arg){
	while(exec){
		printf("%s: I'm alive\n", (char *)arg);
		sleep(1);
	}
}

void * getValue(){
	while(exec){
		int status = sem_wait(sem);
		if (status != 0){
			perror("Error in C2-thread semaphore");
			exit(EXIT_FAILURE);
		}
		printf("Value: %d\n", *shmem);
		if (*shmem == 100){
			pthread_kill(pthread_self(), SIGUSR1);
			exec = false;
		}
	}
}

void hdl(int sig){
	exec = false;      
}
