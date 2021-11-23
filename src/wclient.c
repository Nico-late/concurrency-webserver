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

#define MAXBUF (8192)

pthread_mutex_t lock_client = PTHREAD_MUTEX_INITIALIZER;

void *client_thread(char *argv[]) {
    //a worker thread must wait if the buffer is empty.
    printf("Thread opened\n");
    pthread_mutex_lock(&lock_client); //par défaut le thread crée est blocké
    char *host = argv[1];
    int port = atoi(argv[2]);
    char *filename = argv[3];

    printf(host);
    printf(port);
    printf(filename);

    /* Open a single connection to the specified host and port */
    int clientfd = open_client_fd_or_die(host, port);
    
    client_send(clientfd, filename);
    client_print(clientfd);
    
    close_or_die(clientfd);
    pthread_mutex_unlock(&lock_client);
    pthread_exit(NULL);
}

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
	printf("position 0: %s", buf);
	n = readline_or_die(fd, buf, MAXBUF);
    }
}

int main(int argc, char *argv[]) {
    
    if (argc != 4) {
	fprintf(stderr, "Usage: %s <host> <port> <filename>\n", argv[0]);
	exit(1);
    }
    
    int threads = 4;

    pthread_t pool[threads];
    int i = 0;

    while(i<threads){
        if(pthread_create(&pool[i], NULL, client_thread, argv) != 0 )
            printf("Failed to create client thread\n");
        i++;
    }
}
