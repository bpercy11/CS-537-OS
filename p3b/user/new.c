#include "types.h"
#include "stat.h"
#include "user.h"


int main(int argc, char* argv[]) {


	printf(1, "Inside new.c\n");

	char *p = shmgetat(0, 4);
	*p = 'a';
	int j = shm_refcount(0);
	printf(1, "j after 1 call: %d\n", j);

	char *k = shmgetat(0,4);
	*k = 'b';
	j = shm_refcount(0);
	printf(1, "j after 2 calls: %d\n", j);

	char *m = shmgetat(1,4);
	*m = 'c';
	j = shm_refcount(0);
	printf(1, "j after 3 calls, last to diff. key: %d\n", j);

	//*p = 'a';
	//char* k = shmgetat(1, 2);
    //*k = 'a';
	//char* m = shmgetat(3, 4);
    //*m = 'a';
/*	char* q = shmgetat(0, 3);
	*q = 'a';
	char* r = shmgetat(0, 3);
	*r = 'a';

	char* t = shmgetat(1, 3);
	*t = 'a';
*/	
//	int j = shm_refcount(0);
//	printf(1, "J is: %d\n", j);

   exit();


}
