#include "cs537.h"
#include "request.h"
#include "twrapper.h"

//multi-threaded webserver

void* consumer(void *fd);
int do_get();
void do_put(int arg);
void getargs(int *port, int *threads,int *buffers,int argc, char *argv[]);
void create_threads(int buf_size, int size);

int *buff;       // pointer to the buffer for holding fd's
int numfull = 0;  // how full the buffer *buff is

int fillptr = 0; // where to fill the buffer *bufif
int useptr = 0;  // where to use from the buffer *buff

int ps;          // thread pool size
int bs;          // buffer size

pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t fill = PTHREAD_COND_INITIALIZER;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;


void getargs(int *port, int *threads,int *buffers,int argc, char *argv[])
{
    if (argc != 4) {
    	fprintf(stderr, "Usage: %s <port>\n", argv[0]);
    	exit(1);
    }
    *port = atoi(argv[1]);
    *threads = atoi(argv[2]);
    *buffers = atoi(argv[3]);
		if(*threads < 1 || * buffers < 1) {
			fprintf(stderr, "thread count/buffer count must be >= 1\n");
			exit(1);
		}
}


int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen, buffers, threads;
    struct sockaddr_in clientaddr;


		//printf("1\n");
    getargs(&port, &threads, &buffers, argc, argv);
    //printf("2\n"); 


    // CS537: Create some threads...
    //
		//printf("before createthreads\n");
    create_threads(buffers, threads);
		//printf("after createthreads\n");
    listenfd = Open_listenfd(port);
    while (1) {
      clientlen = sizeof(clientaddr);
      connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);

  // 
  // CS537: In general, don't handle the request in the main thread.
  // Save the relevant info in a buffer and have one of the worker threads 
  // do the work. 
  // 
      Pthread_mutex_lock(&m);
      if(numfull==bs){
        Pthread_mutex_unlock(&m);
        Close(connfd);
      }
      else {
           
        buff[fillptr] = connfd;
        fillptr = (fillptr+1) % bs;
        numfull++;

        pthread_cond_signal(&fill);
        Pthread_mutex_unlock(&m);
      }
    }
}

void* consumer(void *fd){

	while(1){
    int connfd;
    Pthread_mutex_lock(&m);

    while(numfull == 0) {
      pthread_cond_wait(&fill, &m);
		}
    
		connfd = buff[useptr];
		useptr = (useptr+1) % bs;
		numfull--;

    Pthread_mutex_unlock(&m);

    requestHandle(connfd);
  	Close(connfd);
  }
}

void create_threads(int buffsz, int poolsz){

  pthread_t threads[poolsz];

	int j;
  for(j=0;j<poolsz;j++){
      pthread_create(&threads[j], NULL, consumer, NULL);
  }

		 buff = malloc(sizeof(int)*buffsz);

  ps = poolsz;
  bs = buffsz;

}

