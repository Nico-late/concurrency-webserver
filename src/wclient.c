//
// client.c: A very, very primitive HTTP client.
// 
// To run, try: 
//      client hostname portnumber filename
//
// Sends one HTTP request to the specified HTTP server.
// Prints out the HTTP response.
//
// For testing your server, you will want to modify this client.  
// For example:
// You may want to make this multi-threaded so that you can 
// send many requests simultaneously to the server.
//
// You may also want to be able to request different URIs; 
// you may want to get more URIs from the command line 
// or read the list from a file. 
//
// When we test your server, we will be using modifications to this client.
//

#include "io_helper.h"
#include <pthread.h>
#include "spin.c"

#define MAXBUF (8192)

struct thread_info { /* Used as argument to client_thread() */
    char *host;
    char *filename;
    int port;
    int nb_thread;
};

//
// Send an HTTP request for the specified file 
//
void client_send(int fd, char *filename) {
    char buf[MAXBUF];
    char hostname[MAXBUF];
    
    gethostname_or_die(hostname, MAXBUF);
    
    /* Form and send the HTTP request */
    sprintf(buf, "GET %s HTTP/1.1\n", filename);
    sprintf(buf, "%shost: %s\n\r\n", buf, hostname);
    write_or_die(fd, buf, strlen(buf));
}

//
// Read the HTTP response and print it out
//
void client_print(int fd) {
    char buf[MAXBUF];  
    int n;
    
    // Read and display the HTTP Header 
    n = readline_or_die(fd, buf, MAXBUF);
    while (strcmp(buf, "\r\n") && (n > 0)) {
	printf("Header: %s", buf);
	n = readline_or_die(fd, buf, MAXBUF);
	
	// If you want to look for certain HTTP tags... 
	// int length = 0;
	//if (sscanf(buf, "Content-Length: %d ", &length) == 1) {
	//    printf("Length = %d\n", length);
	//}
    }
    
    // Read and display the HTTP Body 
    n = readline_or_die(fd, buf, MAXBUF);
    while (n > 0) {
	printf("%s", buf);
	n = readline_or_die(fd, buf, MAXBUF);
    }
}

pthread_mutex_t lock_client = PTHREAD_MUTEX_INITIALIZER;
    
void *client_thread(void *arg) {
    pthread_mutex_lock(&lock_client);

    struct thread_info *tinfo = arg; 
    char *host = tinfo->host;
    int port = tinfo->port ;
    char *filename =  tinfo->filename;
    int nb_thread = tinfo->nb_thread;
    printf("Thread number %d opened\n", nb_thread);
    //printf("wclient 1 host %s, port %d, filename %s \n", host, port,filename);

    int clientfd = open_client_fd_or_die(host, port);

    client_send(clientfd, filename);
    client_print(clientfd);
    //spin(2);
    close_or_die(clientfd);
    pthread_mutex_unlock(&lock_client);
}


int main(int argc, char *argv[]) {
   
    struct thread_info tinfo;
    printf("Beginning \n");
    if (argc != 5) {
	fprintf(stderr, "Usage: %s <host> <port> <filename> <threads>\n", argv[0]);
	exit(1);
    }
     
    tinfo.host= argv[1];
    tinfo.port= atoi(argv[2]);
    tinfo.filename= argv[3];
    
    /* Open a single connection to the specified host and port */
    
    int nb_threads = atoi(argv[4]);

    pthread_t pool[nb_threads];

    for( int i=0; i<nb_threads; i++){
        tinfo.nb_thread=i;
        if(pthread_create(&pool[i], NULL, client_thread, &(tinfo)) != 0 )
            printf("Failed to create client thread\n");
        pthread_join(pool[i], NULL);        
    }
    exit(0);
}
