#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>

void * sendMsg();
void * getValue();

int main(int argc, char *argv[]){

	int pipefd[2];
	pid_t procA, procB, procC;
	//int buf;
	int value;

	if (pipe(pipefd) == -1) {
		perror("cant create pipe");
		exit(EXIT_FAILURE);
	}
		
	procA = getpid();
	procB = fork();
	
	if(procB != 0){									//Process A
		while(true){ 								
			close(pipefd[0]);
			printf("Input value: ");
			scanf("%d", &value);
			write(pipefd[1], &value, sizeof(value));
			printf("Parent(%d) send value: %d\n", getpid(), value);
			close(pipefd[1]);
			sleep(1);
		}
	}else{ 											//Process B
		while(true){
			close(pipefd[1]);
			read(pipefd[0], &value, sizeof(value));
			printf("Child(%d) received value: %d\n", getpid(), value);
			close(pipefd[0]);
			value *= value;
			sleep(1);
		}
	}
	/*	procC = fork();
		wait(NULL);
		if (procC != 0){ 							//Process B
				
		}else{	
			while(1){								//Process C
			int r1,r2, result;
			pthread_t c1,c2;
			r1 = pthread_create(&c1, NULL, getValue, NULL);
			
			if (r1 != 0){
				perror("Error creating c1 thread");
				return EXIT_FAILURE;
			}
			
			r2 = pthread_create(&c2, NULL, sendMsg, "C2");
			if (r2 != 0){
				perror("Error creating c2 thread");
				return EXIT_FAILURE;
			}
			result = pthread_join(c1, NULL);
			if (result != 0) {
				perror("Joining the second thread");
				return EXIT_FAILURE;
			}
			result = pthread_join(c2, NULL);
			if (result != 0) {
				perror("Joining the second thread");
				return EXIT_FAILURE;
			}
		}
		}
	}*/
	return 0;
}

void * sendMsg(void * arg){
	//char name[] = *(char *)arg;
	printf("%s: I'm alive\n", arg);
	sleep(1);
}

void * getValue(){
	
}
