#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#define BUFLEN 256

int lenHelper(unsigned x) {
    if(x>=1000000000) return 10;
    if(x>=100000000) return 9;
    if(x>=10000000) return 8;
    if(x>=1000000) return 7;
    if(x>=100000) return 6;
    if(x>=10000) return 5;
    if(x>=1000) return 4;
    if(x>=100) return 3;
    if(x>=10) return 2;
    return 1;
}

int printLen(int x) {
    return x<0 ? lenHelper(-x)+1 : lenHelper(x);
}

void error(char *msg) {
	perror(msg);
	exit(-1);
}

int main(int argc, char const *argv[])
{
	char buffer[BUFLEN];
	pid_t proc_id = getpid();

	if (argc != 3) {
		error("[Client]: Server IP or server port is missing!");
	}

	char filename[50];
	memset(filename, 0, 20);
	char pid_str[20];
	int pid_digits = printLen((int)proc_id);

	sprintf(pid_str, "%d", (int)proc_id);

	strcpy(filename, "client-");
	strcpy(filename + strlen(filename), pid_str);
	strcpy(filename + strlen(filename), ".log");

	FILE *fp;
	fp = fopen(filename, "w+");


	/* Init data. */
// ------------------------------------------------------------------ //
	int sockfd = 0;
    int i, n;
    struct sockaddr_in serv_addr; 

    fd_set read_fds;
    fd_set tmp_fds; 
    int fdmax; 
    
    FD_ZERO(&read_fds);
    FD_ZERO(&tmp_fds);
// ------------------------------------------------------------------ //
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
        error("[Client]: Error opening socket.");

	/* Complete info about the client. */
// ------------------------------------------------------------------ //
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[2]));
	inet_aton(argv[1], &serv_addr.sin_addr);

	if(connect(sockfd,(struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) < 0){
		error("[Client]: Error on connection.");
	}

	FD_SET(sockfd, &read_fds);
    FD_SET(0, &read_fds);
    fdmax = sockfd;
// ------------------------------------------------------------------ //

    int closed = 0;
	while (1) {
        tmp_fds = read_fds; 
        if (select(fdmax + 1, &tmp_fds, NULL, NULL, NULL) == -1) 
            error("[Client]: Error in select!");
        for(i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &tmp_fds)) {
                if(i != 0) {
                    memset(buffer, 0, BUFLEN);
                    if ((n = recv(i, buffer, sizeof(buffer), 0)) <= 0) {
                        if (n == 0) {
                            printf("[Client]: Socket %d hung up\n", i);
                            closed = 1;
                            
                            break;
                        } else {
                            error("[Client]: Error in recv!");
                        }

                        close(i); 
                        FD_CLR(i, &read_fds);
                    } else {
                    	/* Print buffer to file if arrived. */
                    	fprintf(fp, "%s\n", buffer);
                        puts(buffer);
                    }
                } else {
                    memset(buffer, 0 , BUFLEN);
                    sprintf(buffer, "%d", (int)proc_id);
                    fgets(buffer + strlen(buffer), BUFLEN-1, stdin);
                    /* Print buffer to file before sending. */
                    fprintf(fp, "%s\n", buffer + pid_digits);

                    /* Sending message to server. */
                    n = send(sockfd, buffer, strlen(buffer), 0);
                    if (n < 0) 
                        error("[Client]: Error writing to socket!");
                }
            }
        }
    	if (closed)
    		break;
    }
    close (sockfd);
    // fclose(fp);
	return 0;
}