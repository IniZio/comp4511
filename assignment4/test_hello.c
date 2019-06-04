#include <stdio.h>
#include <unistd.h>

#define SYS_HELLO_NUM  335

int main() {
	int res = syscall(SYS_HELLO_NUM);
	printf("The return value is %d\n", res); 
	return 0;
}

