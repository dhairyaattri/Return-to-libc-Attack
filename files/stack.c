#include<stdio.h>
#include<string.h>
int vul_func(char *str)
{
	char buffer[50];
	strcpy (buffer, str);
	return 1;
}

int main (int argc, char **argv)
{
	char str[240];
	FILE *badfile;

	badfile = fopen ("badfile", "r");
	fread(str, sizeof(char), 200, badfile);
	vul_func(str);

	printf("Returned Properly\n");
	return 1;
}
