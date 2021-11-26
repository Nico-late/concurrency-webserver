#include <stdio.h>
#include "request.h"
#include "io_helper.h"
#include <pthread.h>
#include "spin.c"

char default_root[] = ".";

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t master_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t needToEmpty = PTHREAD_COND_INITIALIZER;
pthread_cond_t needToFill = PTHREAD_COND_INITIALIZER;

struct {
	int count;
	int buff_size;
	int nb_threads;
	int buff[];
} shared = {0};

int get_fd () {
	shared.count --;
	int fd = shared.buff[shared.count];
	if (shared.count == 0) {
		printf("buffer empty\n");
		pthread_cond_signal(&needToFill);
	}
	else {
		pthread_cond_signal(&needToEmpty);
	}
	return fd;
}
void put_fd (int fd) {
	shared.buff[shared.count]=fd;
	shared.count ++;
	if (shared.count == shared.buff_size) {
		printf("buffer full\n");
		pthread_cond_signal(&needToEmpty);
	}
	pthread_cond_signal(&needToEmpty);
	
}

void *worker_thread_consumer(void *arg) {
	while (1){
		int nb = *((int *)arg);
		printf("inside worker thread number %d",nb);
		pthread_mutex_lock(&lock); //par défaut le thread crée est blocké
		pthread_cond_wait (&needToEmpty, &lock);
		int fd = get_fd();
		request_handle(fd);
		spin(2);
		close_or_die(fd);
		pthread_mutex_unlock(&lock);
	}
}

void master_thread_producer (void * arg) {
	int fd = *((int *)arg);
	pthread_mutex_lock(&master_lock);
	while (shared.count == shared.buff_size) {
		printf("buffer full master");
		pthread_cond_wait(&needToFill, &lock);
	}
	put_fd(fd);
	pthread_mutex_unlock(&master_lock);
}

/*"""Note that the master thread and the worker threads are in a producer-consumer
relationship and require that their accesses to the shared buffer be
synchronized. Specifically, the master thread must block and wait if the
buffer is full; a worker thread must wait if the buffer is empty. In this
project, you are required to use condition variables. Note: if your
implementation performs any busy-waiting (or spin-waiting) instead, you will
be penalized (i.e., do not do that!)."""*/

//
// ./wserver [-d <basedir>] [-p <portnum>] [-t <threads>] [-b <buffers>]
// 
int main(int argc, char *argv[]) {
    int c;
    char *root_dir = default_root;
    int port = 10000;
	int threads = 10;
	int buffers = 40;
    
    while ((c = getopt(argc, argv, "d:p:t:b:")) != -1)
	switch (c) {
	case 'd':
	    root_dir = optarg;
	    break;
	case 'p':
	    port = atoi(optarg);
	    break;
	case 't':
		threads = atoi(optarg);
		if (threads<=0){
	    	fprintf(stderr, "Number of threads has to be a positive integer\n");
			exit(1);
		}
		break;
	case 'b':
		buffers = atoi(optarg);
		if (buffers<=0){
			fprintf(stderr, "Number of buffers has to be a positive integer\n");
			exit(1);
		}
		break;
	default:
	    fprintf(stderr, "usage: wserver [-d basedir] [-p port] [-t threads] [-b buffers]\n");
	    exit(1);
	}

    // run out of this directory
    chdir_or_die(root_dir);

	//create buffer
	//extern int buff[buffers];

	//fill the structure with infos
	shared.buff_size = buffers;
	shared.nb_threads = threads;
  

    // create pool of threads before the while
	pthread_t pool[threads];
	printf("nb of threads = %d \n", threads);
    for (int i=0; i<threads; i++) {
        if( pthread_create(&pool[i], NULL, worker_thread_consumer, &i) != 0 )
           printf("Failed to create thread\n");
    }

    // now, get to work
    int listen_fd = open_listen_fd_or_die(port);

    while (1) {
		struct sockaddr_in client_addr;
		int client_len = sizeof(client_addr);
		int conn_fd = accept_or_die(listen_fd, (sockaddr_t *) &client_addr, (socklen_t *) &client_len);
		master_thread_producer(&conn_fd);		
	}
    return 0;
}