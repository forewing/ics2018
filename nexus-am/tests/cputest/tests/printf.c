#include "trap.h"

char buf[128];

void printstr(char* str){
	int i;
	for (i = 0; i < strlen(str); i++){
		_putc(str[i]);
	}
}

int main() {
	sprintf(buf, "fuck %d %s %d\n", 100, "haha", -100);
	int ret;
	ret = printf(buf);
	printf("%d\n", ret);
	printf("%d\n", strlen(buf));
	// printf("%d\n", strcmp(buf, "fuck 100 haha -100\n"));
	printstr(buf);
	return 0;
}
//2