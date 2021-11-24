#include <stdio.h>
#include "request.h"
#include "io_helper.h"
#include <pthread.h>

char default_root[] = ".";

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void *server_thread(void *arg) {
    //a worker thread must wait if the buffer is empty.
    int fd = *((int *)arg);
    printf("fd = %d. \n", fd);
	pthread_mutex_lock(&lock); //par défaut le thread crée est blocké

    request_handle(fd);

	pthread_mutex_unlock(&lock);

    close_or_die(fd);
    pthread_exit(NULL);
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
	int buffers = 1;
    
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

	pthread_t pool[threads];
	int i=0;

    // now, get to work
    int listen_fd = open_listen_fd_or_die(port);
    while (1) {
		struct sockaddr_in client_addr;
		int client_len = sizeof(client_addr);
		int conn_fd = accept_or_die(listen_fd, (sockaddr_t *) &client_addr, (socklen_t *) &client_len);
		if(pthread_create(&pool[i++], NULL, server_thread, &conn_fd) != 0 ){
			printf("Failed to create thread\n");
		}

		// Solution trouvée sur internet pour vérifier que tous les threads
		// dans la pool sont fermés avant de relancer des threads...
		// Mais ça me parait pas optimal
		if( i >= threads)
        {
          i = 0;
          while(i < threads)
          {
            pthread_join(pool[i++],NULL);
          }
          i = 0;
        }
	}

    return 0;
}