#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>


static void print_sigset(const sigset_t *set)
{
    int sig, cnt;

    cnt = 0;
    printf("signal set:\n");
    for (sig = 1; sig < NSIG; sig++) {
        if (sigismember(set, sig)) {
            printf("\t%2d (%s)\n", sig, strsignal(sig));
            cnt++;
        }
    }

    if (cnt == 0)
        printf("\t<empty>\n");
}

void sig_handler1(int signum) 
{
	printf("Sign handler 1\n");
	//exit(EXIT_SUCCESS);
}

void sig_handler2(int signum)
{
        printf("Sign handler 2\n");
        //exit(EXIT_SUCCESS);
}


static void *handle_th1(void *args) 
{ 
    //sleep(1);
    sigset_t set, pending;

    if (signal(SIGINT, sig_handler1) == SIG_ERR) {
            fprintf(stderr, "Cannot handle SIGINT\n");
            exit(EXIT_FAILURE);
    }    

    sigemptyset(&set);

    sigaddset(&set, SIGINT);
   
    pthread_sigmask(SIG_BLOCK, &set, NULL);
    
    sleep(20);

    sigpending(&pending);
    print_sigset(&pending);

    pthread_exit(NULL); // exit

}

static void *handle_th2(void *args) 
{
    sigset_t set, pending;
    
    if (signal(SIGCHLD, sig_handler1) == SIG_ERR) {
            fprintf(stderr, "Cannot handle SIGINT\n");
            exit(EXIT_FAILURE);
    }

    sigemptyset(&set);

    sigaddset(&set, SIGCHLD);

    pthread_sigmask(SIG_BLOCK, &set, NULL);


    sleep(20);

    sigpending(&pending);
    print_sigset(&pending);

    pthread_exit(NULL); // exit
}

static void *handle_th3(void *args)
{ 
    pthread_t *thread_id1 = (pthread_t *)args;
    for(int i=0; i<5; i++) {
        pthread_kill(*thread_id1, SIGINT);
	//pthread_kill(*thread_id2, SIGCHLD);
	sleep(1);
    }
    //pthread_exit(NULL);
}

int main(int argc, char const *argv[])
{
    /* code */
    int ret;

    pthread_t thread_id1, thread_id2, thread_id3;

    if (ret = pthread_create(&thread_id1, NULL, &handle_th1, NULL)) {
        printf("pthread_create() error number=%d\n", ret);
        return -1;
    }

    //pthread_kill(thread_id1, SIGINT);
	
    if (ret = pthread_create(&thread_id2, NULL, &handle_th2, NULL)) {
        printf("pthread_create() error number=%d\n", ret);
        return -1;
    }
    
    if (ret = pthread_create(&thread_id3, NULL, &handle_th3, &thread_id1)) {
        printf("pthread_create() error number=%d\n", ret);
        return -1;
    }



    // used to block for the end of a thread and release
    pthread_join(thread_id1,NULL);  
    pthread_join(thread_id2,NULL);
    pthread_join(thread_id3,NULL);

    return 0;
}
