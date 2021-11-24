#include <stdio.h>
#include "request.h"
#include "io_helper.h"

char default_root[] = ".";

//
// ./wserver [-d <basedir>] [-p <portnum>] 
// 
int main(int argc, char *argv[]) {
    int c;
    char *root_dir = default_root;
    int port = 10000;
    
    while ((c = getopt(argc, argv, "d:p:")) != -1)
	switch (c) {
	case 'd':
	    root_dir = optarg;
	    break;
	case 'p':
	    port = atoi(optarg);
	    break;
	default:
	    fprintf(stderr, "usage: wserver [-d basedir] [-p port]\n");
	    exit(1);
	}

    // run out of this directory
    chdir_or_die(root_dir);

    // now, get to work
    int listen_fd = open_listen_fd_or_die(port);
    while (1) {
    	printf("0 \n");
	struct sockaddr_in client_addr;
	int client_len = sizeof(client_addr);
	printf("0.5 \n");
	int conn_fd = accept_or_die(listen_fd, (sockaddr_t *) &client_addr, (socklen_t *) &client_len);
	printf(" 1 \n");
	request_handle(conn_fd);
	printf("2 \n");
	close_or_die(conn_fd);
	printf("3 \n");
    }
    return 0;
}
