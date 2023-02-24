
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void foo()
{
    int a;
    int *p = &a;
    *p = 1;
    printf("*p = %d\n", *p);
    free(p);
}
 
int main()
{
    printf("PID: %d", getpid());
    foo();
    return 0;
}
