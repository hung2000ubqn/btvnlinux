#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>

void sig_handler1(int signum) 
{
	printf("Sign handler 1\n");
	//exit(EXIT_SUCCESS);
}

void sig_handler2(int signum)
{
        printf("Sign handler 2\n");
        exit(EXIT_SUCCESS);
}


static void *handle_th1(void *args) 
{   
    //sleep(1);
    sigset_t new_set, old_set;

    if (signal(SIGINT, sig_handler1) == SIG_ERR) {
            fprintf(stderr, "Cannot handle SIGINT\n");
            exit(EXIT_FAILURE);
    }    

    sigemptyset(&new_set);
    sigemptyset(&old_set);

    sigaddset(&new_set, SIGINT);
    if (sigprocmask(SIG_SETMASK, &new_set, &old_set) == 0) {
            if (sigismember(&new_set, SIGINT) == 1 ) {
                     printf("SIGINT was blocked\n");
            } else if (sigismember(&new_set, SIGINT) == 0) {
                     printf("SIGINT wasn't blocked\n");
            }
    }
    while(1);

    //pthread_exit(NULL); // exit

}

static void *handle_th2(void *args) 
{
    sigset_t new_set, old_set;
    
    if (signal(SIGCHLD, sig_handler1) == SIG_ERR) {
            fprintf(stderr, "Cannot handle SIGINT\n");
            exit(EXIT_FAILURE);
    }

    //sigemptyset(&new_set);
    //sigemptyset(&old_set);

    sigaddset(&new_set, SIGCHLD);
    if (sigprocmask(SIG_SETMASK, &new_set, &old_set) == 0) {
            if (sigismember(&new_set, SIGCHLD) == 1 ) {
                     printf("SIGCHLD was blocked\n");
            } else if (sigismember(&new_set, SIGCHLD) == 0) {
                     printf("SIGCHLD wasn't blocked\n");
            }
    }
    while(1);


    //pthread_exit(NULL); // exit
}

int main(int argc, char const *argv[])
{
    /* code */
    int ret;
    pthread_t thread_id1, thread_id2;
    printf("1\n");


    if (ret = pthread_create(&thread_id1, NULL, &handle_th1, NULL)) {
        printf("pthread_create() error number=%d\n", ret);
        return -1;
    }

    //pthread_kill(thread_id1, SIGINT);
    printf("2\n");
    sleep(5);
	
    if (ret = pthread_create(&thread_id2, NULL, &handle_th2, NULL)) {
        printf("pthread_create() error number=%d\n", ret);
        return -1;
    }
    
    while(1) 
    {
	    pthread_kill(thread_id1, SIGINT);
	    pthread_kill(thread_id2, SIGCHLD);
	    sleep(1);
    }

    // used to block for the end of a thread and release
    //pthread_join(thread_id1,NULL);  
    //pthread_join(thread_id2,NULL);

    return 0;
}
