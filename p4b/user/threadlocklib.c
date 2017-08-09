#include "types.h"
#include "stat.h"
#include "fcntl.h"
#include "user.h"
#include "x86.h"


//thread library

#define SINGLEPAGE 4096

//typedef struct __lock_t {
//  uint locked;
//  uint guard;
//} lock_t;


void 
lock_acquire(lock_str *lock) {
  while(xchg(&lock->held, 1));
}

void 
lock_release(lock_str *lock) {
  lock->held = 0;
}

void 
lock_init(lock_str *lock) {
 lock->held = 0;
 lock->guarded = 0;
}

int
thread_create(void (*start_routine)(void*), void *arg) {

  void *st = malloc(SINGLEPAGE * 2);
  if((uint)st % SINGLEPAGE)
    st = st + (SINGLEPAGE - (uint)st % SINGLEPAGE);
  
  int thread_id = clone(start_routine, arg, st);
  return thread_id;
}

int
thread_join() {

  void *u = NULL;
  int i = join(&u);
  free(u);
  return i;

}
