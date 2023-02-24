#include <stdio.h>
#include <unistd.h>

int main() 
{
	printf("Test\n");
	printf("PID of Test: %d\n", getpid());
	return 0;
}
