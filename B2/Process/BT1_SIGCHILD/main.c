#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

void sig_handler(int signum) {
	printf("sig handler\n");
	exit(EXIT_SUCCESS);
}

int main(int argc, char const *argv[])   
{

	pid_t processA;                
	int status, retv;

	processA = fork();         
	if (processA >= 0) {
        	if (0 == processA) {
			//Child Process	
        		printf("\nDay la tien trinh con B\n");
		 	sleep(5);	
        	} else {               
	    		//Parent Process
            		sigset_t new_set, old_set;
	
			if (signal(SIGCHLD, sig_handler) == SIG_ERR) {
                		fprintf(stderr, "Cannot handle SIGINT\n");
                		exit(EXIT_FAILURE);
        		}
	
        		sigemptyset(&new_set);
			sigemptyset(&old_set);

        		//sigaddset(&new_set, SIGCHLD);

        		if (sigprocmask(SIG_SETMASK, &new_set, &old_set) == 0) {
				if (sigismember(&new_set, SIGCHLD) == 1 ) {
					printf("SIGCHLD was blocked\n");
				} else if (sigismember(&new_set, SIGCHLD) == 0) {
					printf("SIGCHLD wasn't blocked\n");
				}
			}

			retv = wait(&status);
                        if (retv == -1) {
                                printf("wait () unsuccessfull\n");
                        }

			//while(1);
       		} 
	} else {
        	printf("fork khong thanh cong\n");
    		exit(EXIT_FAILURE);	
    	}

    	return 0;
}
