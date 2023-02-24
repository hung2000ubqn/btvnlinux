#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

char *sys_argv[] = {"ls","-l","-a","-h"};

void execl_demo() {
	printf("Run command <ls -lah> after 2 seconds\n");
    	sleep(2);
    	execl("/bin/ls", *sys_argv, *(sys_argv+1), *(sys_argv+2), *(sys_argv+3), NULL);
}

void execlp_demo() {
	printf("Run command <ls -lah> after 2 seconds\n");
    	sleep(2);
    	execlp("ls", *sys_argv, *(sys_argv+1), *(sys_argv+2), *(sys_argv+3), NULL);
}

void execvp_demo() {
	printf("Run command <ls -lah> after 2 seconds\n");
        sleep(2);
        execvp("ls", sys_argv);
}

void execv_demo() {
	printf("Run command <ls -lah> after 2 seconds\n");
        sleep(2);
        execv("/bin/ls", sys_argv);
}


int main(int argc, char *argv[]) 
{    
    //execl_demo();
    //execlp_demo();
    //execvp_demo();
    //execv_demo();
    pid_t retv;
    retv = fork();
    if (retv == 0) {
	    printf("Child process\n");
	    printf("Child ID: %d\n", getpid());
	    printf("Run test application\n");
	    char *args[] = {"./test", NULL};
	    execv(args[0], args);
    } else if (retv > 0) {
	    //Parent process
	    printf("Parent process\n");
	    printf("Parent ID: %d\n", getpid());
	    while(1);
    } else {
	    printf("Fork unsuccessful\n");
    }
    return 0;   
}

