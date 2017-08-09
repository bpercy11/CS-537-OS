#ifndef __TWRAPPER_h__
#define __TWRAPPER_h__
#include <pthread.h>

void
Pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void*), void *arg) {
    int rc = pthread_create(thread, attr, start_routine, arg);
		rc++;
}

void
Pthread_join(pthread_t thread, void **value_ptr) {
    int rc = pthread_join(thread, value_ptr);
		rc++;
}



void
Pthread_mutex_lock(pthread_mutex_t *m) {
    int rc = pthread_mutex_lock(m);
		rc++;
}

void
Pthread_mutex_unlock(pthread_mutex_t *m) {
    int rc = pthread_mutex_unlock(m);
		rc++;
}


#endif // __TWRAPPER_h__
