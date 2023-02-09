#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char const *argv[])   
{

	pid_t processA, w;                
	int status;

	processA = fork();         
	if (processA >= 0) {
        	if (0 == processA) {
			//Tien trinh con B	
        		printf("\nDay la tien trinh con B\n");
            		printf("PID: %d, PPID: %d\n", getpid(), getppid());
		    	while(1);	
        	} else {               
	    		//Tien trinh A
            		printf("\nDay la tien trinh A\n");
            		printf("PID: %d\n", getpid());
	    		while (1) {
	       			w = waitpid(processA, &status, WNOHANG);
	       			if (w > 0) {
					if (WIFEXITED(status)) {
						printf("Process ket thuc binh thuong, trang thai = %d\n",WEXITSTATUS(status));
					} else if (WIFSIGNALED(status)) {
                       				printf("Process bi ket thuc boi signal %d\n", WTERMSIG(status));
                   			} else if (WIFSTOPPED(status)) {
                       				printf("Process bi dung boi signal %d\n", WSTOPSIG(status));
                   			} else if (WIFCONTINUED(status)) {
                       				printf("Process hoat dong binh thuong\n");
					}
				} else if (w == -1) {
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
