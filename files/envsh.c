#include <stdio.h>
int main(){
	char *shell = (char *)getenv("MYSHELL");

	if(shell){
		printf("Value: %s\n", shell);
		printf("Address: %x\n", (unsigned int)shell);
	}

	return 1;
}
