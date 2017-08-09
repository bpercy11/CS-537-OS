#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"


int
main(int argc, char *argv[])
{

	int firstcall = getnumsyscallp();

	int argnum  = atoi(argv[1]);
	int i;
	for(i = 0; i<argnum; i++) {
		getpid();
	}

	int secondcall = getnumsyscallp();

	printf(1, "%d %d\n", firstcall, secondcall);
	exit();
}
