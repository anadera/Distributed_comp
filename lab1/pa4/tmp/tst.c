#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#define MAX_PROC 10

typedef struct cmArgs{
  int mutexIsUsed;
  int x;
} CM_ARGUMENTS;
char *optarg;

int parse_cm(int argc, char** argv, CM_ARGUMENTS* cmArgs){

	int option_index = 0;
	int opt = 0;
	static struct option longOpt[] = {
		{"proc", required_argument, 0, 'p'},
                {"mutexl", no_argument, 0, 'm'},
		{NULL, 0, 0, 0}
        };

	while (( opt = getopt_long(argc, argv, "p:m", longOpt, &option_index)) > 0) {
		switch (opt) {
		case 'm':
			cmArgs->mutexIsUsed = 1;
			break;
		case 'p': 
			cmArgs->x = atoi(optarg);
			if ( cmArgs->x > MAX_PROC || cmArgs->x <= 0)
				return -1;
			break;
		
		default:
			break;
		}
	}
	return cmArgs->x;
}

int main(int argc, char** argv){
	int d;
	CM_ARGUMENTS* cmArgs;
	cmArgs = malloc(sizeof(cmArgs));
	//cmArgs->mutexIsUsed = 0;
	//cmArgs->x = 0;
	d = parse_cm(argc,argv, cmArgs);
	printf ("mutexIsUsed = %d\n", cmArgs->mutexIsUsed);
	printf ("x = %d\n", cmArgs->x);
	return 0;
}
