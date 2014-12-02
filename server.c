/* csci4061 F2013 Assignment 4 
* section: one_digit_number 
* date: mm/dd/yy 
* names: Name of each member of the team (for partners)
* UMN Internet ID, Student ID (xxxxxxxx, 4444444), (for partners)
*/

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "util.h"

#define MAX_THREADS 100
#define MAX_QUEUE_SIZE 100
#define MAX_REQUEST_LENGTH 64
#define MAX_CACHE_SIZE 100

typedef struct request_queue
{
	int		m_socket;
	char	m_szRequest[MAX_REQUEST_LENGTH];
} request_queue_t;


typedef struct requests
{
	int fd;
	char filename[1024]; 
	struct requests *next; 
	struct requests *prev;
} request ;

request * linkhead;
request *currentReq;
int num_dispatch;
int num_workers;
int num_prefetch ;
int max_queue_size;
 int max_cache_size;
	
/************locks variables******/
pthread_mutex_t queueLock=PTHREAD_MUTEX_INITIALIZER; //lock whenever operation for queue performs

/************Locks variables*********/



void insertRequest(request *req){
	if(linkhead==NULL){
		linkhead= req;
		currentReq=linkhead;
	}
	else {
		currentReq=linkhead;
		while(currentReq->next!=NULL){
			currentReq=currentReq->next;
		}
		currentReq->next=req;
		
		
	}
}


struct request* createRequest(int fd, const char* const fn) {
	request* req = (request*)malloc( sizeof(request) );
	
	req->fd = fd;
	strncpy(req->filename, fn, 1024);

	req->next = NULL;
	req->prev = NULL;

	return req;
}

void * dispatch(void * arg)
{
	int fd;
	char filename_buffer[1024];
	char* filename;
	request* req;

	fprintf(stderr, "dispatch_thread: Starting up\n");

	while ( (fd = accept_connection()) >= 0 ) {
			fprintf(stderr,"dispatch_thread: Got connection: %d\n", fd);
			if (get_request(fd, filename_buffer) == 0) {
					fprintf(stderr, "dispatch_thread: Got request for: %s\n", filename_buffer);

					if (filename_buffer[0] == '/') {
							fprintf(stderr, "dispatch_thread: Trimming leading / from filename\n");

							filename = &filename_buffer[1];
					} else
							filename = &filename_buffer[0];

					fprintf(stderr, "dispatch_thread: Acting on file: %s\n", filename);

					req = createRequest(fd, filename);

					pthread_mutex_lock(&queueLock);
					//if (queue_size == max_queue_size)
						//	pthread_cond_wait(&queue_put_cond, &queueLock);
					insertRequest(req);

					//pthread_cond_signal(&queue_get_cond);

					pthread_mutex_unlock(&queueLock);
			}
	}

	fprintf(stderr, "dispatch_thread: Shutting down\n");

	return NULL;
}

void * worker(void * arg)
{
	return NULL;
}
void *prefetch_thread(void *arg){
	return NULL;
}

int main(int argc, char **argv)
{
	if(argc != 9)
	{
		printf("usage: %s port path num_dispatcher num_workers queue_length [cache_size]\n", argv[0]);
		return -1;
	}

    init( atoi( argv[1] ) ); //accept port number and call init()

    if ( chdir( argv[2] ) != 0 ) {
        fprintf(stderr, "Error! Could not set the web tree root directory to %s", argv[2]);
        exit(1);
    }

     num_dispatch = atoi( argv[3] );
    if (num_dispatch <= 0 || num_dispatch > MAX_THREADS) {
        fprintf(stderr, "Error! Invalid number of dispatch threads (Max is %d)\n", MAX_THREADS);
        exit(1);
    }
    
     num_workers = atoi( argv[4] );
    if (num_workers <= 0 || num_workers > MAX_THREADS) {
        fprintf(stderr, "Error! Invalid number of worker threads (Max is %d)\n", MAX_THREADS);
        exit(1);
    }

     num_prefetch = atoi( argv[5] );
    if (num_prefetch <= 0 || num_prefetch > MAX_THREADS) {
        fprintf(stderr, "Error! Invalid number of prefetch threads. (Max is %d)\n", MAX_THREADS);
        exit(1);
    }

     max_queue_size = atoi( argv[6] );
    if (max_queue_size <= 0 || max_queue_size >  MAX_QUEUE_SIZE) {
        fprintf(stderr, "Error! Invalid request queue length (Max length is %d)\n", MAX_QUEUE_SIZE);
        exit(1);
    }

    // argv[7] := <mode>
    // The mode (FCFS, CRF, SFF) to run the worker thread(s) in
    // FCFS: First Come First Serve
    // CRF: Cached Requests First
    // SFF: Smallest File First
    int m = atoi( argv[7] );
	max_cache_size = atoi( argv[8] );
    if (max_cache_size <= 0 || max_cache_size > MAX_CACHE_SIZE) {
        fprintf(stderr, "Error! Invalid cache size (Max size is %d)\n", MAX_CACHE_SIZE);
        exit(1);
    }
    request * linkhead=(request*)malloc( sizeof(request) );
	request *currentReq=(request*)malloc( sizeof(request) );

    //logfile = fopen("web_server_log", "a");

    //assert( logfile != NULL );



    pthread_t dispatch_threads[num_dispatch];
    pthread_t worker_threads[num_workers];
    pthread_t prefetch_threads[num_prefetch];

    int worker_thread_ids[num_workers];
    int i;

    for (i = 0; i < num_workers; ++i)
        worker_thread_ids[i] = i + 1;

    for (i = 0; i < num_dispatch; ++i)
        pthread_create(&dispatch_threads[i], NULL, &dispatch, NULL);
    for (i = 0; i < num_workers; ++i)
        pthread_create(&worker_threads[i], NULL, &worker, (void*)&worker_thread_ids[i]);
    for (i = 0; i < num_dispatch; ++i)
        pthread_join(dispatch_threads[i], NULL);
	for (i = 0; i < num_prefetch; ++i)
	pthread_create(&prefetch_threads[i], NULL, &prefetch_thread, NULL);


    int global_exit = 1;

    for (i = 0; i < num_workers; ++i)
        pthread_join(worker_threads[i], NULL);
	for (i = 0; i < num_prefetch; ++i)
	pthread_join(prefetch_threads[i], NULL);
    printf("Shutting down\n");


	printf("Call init() first and make a dispather and worker threads\n");
	return 0;
}
