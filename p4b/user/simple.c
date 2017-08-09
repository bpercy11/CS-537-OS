#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  int j;
	j = clone(NULL, NULL, NULL);

	

	printf(1, "%d\n", j);


	exit();
}
