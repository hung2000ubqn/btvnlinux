#include <stdio.h>
#include "hello.h"
#include "linux.h"
#include "sum.h"

void main() 
{
	hello();
	hellolinux();
	printf("Tong 2 so %d va %d bang %d\n", 3, 4, sum(3,4));

}
