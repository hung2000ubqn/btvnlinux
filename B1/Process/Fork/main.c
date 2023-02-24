#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char const *argv[])   
{

	pid_t processA, retv;                
	int status;

	processA = fork();         
	if (processA >= 0) {
        	if (0 == processA) {
			//Tien trinh con B	
        		printf("\nChild Process B\n");
            		printf("PID: %d, PPID: %d\n", getpid(), getppid());
		    	while(1);	
        	} else {               
	    		//Tien trinh A
            		printf("\nParent Process A\n");
            		printf("PID: %d\n", getpid());
	    		while (1) {
	       			retv = waitpid(processA, &status, WNOHANG);
	       			if (w > 0) {
					if (WIFEXITED(status)) {
						printf("Process terminated normally, trang thai = %d\n",WEXITSTATUS(status));
					} else if (WIFSIGNALED(status)) {
                       				printf("Process is killed by signal %d\n", WTERMSIG(status));
                   			} else if (WIFSTOPPED(status)) {
                       				printf("Process is stopped by signal %d\n", WSTOPSIG(status));
                   			} else if (WIFCONTINUED(status)) {
                       				printf("Process run normally\n");
					}
				} else if (retv == -1) {
					printf("waitpid");
					exit(EXIT_FAILURE);
				}
			}
		//exit(EXIT_SUCCESS);
		} 
	} else {
        	printf("fork khong thanh cong\n");
    		exit(EXIT_FAILURE);	
    	}

    	return 0;
}
