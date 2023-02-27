#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define BUFF_SIZE 32

#define OUTOFBOUNDS
#undef OUTOFBOUNDS

void sig_handler(int signum) {
    printf("Signal Handler\n");
    exit(EXIT_SUCCESS);
}

void print_set_bin(sigset_t *setp) 
{
	int sig, res;
	char buff[BUFF_SIZE];

	if (!setp) {
		fprintf(stderr, "print_set_bin(): NULL parameter \n");
		return;
	}

#ifdef OUTOFBOUNDS
	for (sig = 0; sig <= NSIG; sig++)
#else
	for (sig = 1; sig <= NSIG; sig++)
#endif 
	{
		res = sigismember(setp, sig);
		if (res == -1) {
			snprintf(buff, BUFF_SIZE, "sigismember [%d]", sig);
			perror(buff);
		} else 
			printf("%d", res);
	}
	printf(" [%s]\n\n", sigemptyset(setp) ? "Empty" : "Non-empty");
}


int main() {
	//time_t start, finish;
  	sigset_t new_set, old_set;
	
	if (signal(SIGINT, sig_handler) == SIG_ERR) {
                fprintf(stderr, "Cannot handle SIGINT\n");
                exit(EXIT_FAILURE);
        }
	
        sigemptyset(&new_set);
	sigemptyset(&old_set);

        sigaddset(&new_set, SIGINT);
 	//printf("new_set is %x\n", new_set);

        if (sigprocmask(SIG_SETMASK, &new_set, &old_set) == 0) {
	 	sigprocmask(SIG_SETMASK, NULL, &old_set);
		if (sigismember(&new_set, SIGINT) == 1 ) {
			printf("SIGINT was blocked\n");
		} else if (sigismember(&new_set, SIGINT) == 0) {
			printf("SIGINT wasn't blocked\n");
		}
       	}
	printf("Signal mask is: \n");
	print_set_bin(&old_set);

	while (1);
	
				
	return 0;
}
