#include "io_helper.h"
#include <pthread.h>

#define MAXBUF (8192)

struct thread_info { /* Used as argument to client_thread() */
    char *host;
    char *filename;
    int port;
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
    struct thread_info *tinfo = arg; 
    char *host = tinfo->host;
    int port = tinfo->port ;
    char *filename =  tinfo->filename;
    printf("Thread opened\n");
    int clientfd = open_client_fd_or_die(host, port);

    client_send(clientfd, filename);
    client_print(clientfd);
    close_or_die(clientfd);    
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

    // cree un nombre de threads defini par l'utilisateur, puis les join
    pthread_t pool[nb_threads];

    for( int i=0; i<nb_threads; i++){
        if(pthread_create(&pool[i], NULL, client_thread, &(tinfo)) != 0 )
            printf("Failed to create client thread\n");
               
    }
    for( int i=0; i<nb_threads; i++){
        pthread_join(pool[i], NULL); 
    }
    exit(0);
}
