#include "types.h"
#include "stat.h"
#include "user.h"


int main() {


	printf(1, "Inside new.c\n");

	char* p = shmgetat(0, 1);

	printf(1, "shared mem print from new1: %p\n", *p);

//	*p = 'a';
/*	char* q = shmgetat(0, 3);
	*q = 'a';
	char* r = shmgetat(0, 3);
	*r = 'a';

	char* t = shmgetat(1, 3);
	*t = 'a';
*/	
	int j = shm_refcount(0);
	printf(1, "J is: %d\n", j);

	return 0;


}
