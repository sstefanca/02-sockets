#include <stdio.h>

// hton, ntoh
#include <arpa/inet.h>

//socket, bind, listen
#include <sys/types.h>
#include <sys/socket.h>

//open, splice
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//strcpy
#include <string.h>

//pipe
#include <unistd.h>

#include "header.h"

#define QUEUE_LIMIT 50
#define BUF_SIZE 128

int count;

int main(int argc, char **argv)
{
    int sockfd;
    int port, ret;
    struct sockaddr_in sockaddr;
    printf("This is the client\n");
    if(argc != 4)
    {
	fprintf(stderr, "Usage: %s <ip> <port> <path>\n", argv[0]);
	return 1;
    }

    ret = sscanf(argv[2], "%i", &port);
    if(ret != 1 || port > 0xffff || port < 1024)
    {
	fprintf(stderr, "Usage: Port number must be between %i and %i.\n", 0xffff, 1024);
	return 2;
    }

    printf("%i\n", port);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);
    sockaddr.sin_addr.s_addr = inet_addr(argv[1]);

    ret = connect(sockfd, (struct sockaddr *)&sockaddr, sizeof(sockaddr));
    if(ret != 0)
    {
	fprintf(stderr, "Error connecting to server!\n");
	return 3;
    }
    
    //while(1)
    {
	header_t head;

	head.msg = REQUEST;
	strcpy(head.path, argv[3]);

	send(sockfd, &head, sizeof(head), 0);
	recv(sockfd, &head, sizeof(head), 0);

	if(head.msg == ACK)
	{
	    int fd;

	    fd = open(argv[3], O_CREAT | O_WRONLY | O_TRUNC);
	    close(fd);
	    close(sockfd);
	}
		
	//TODO: request file
	//TODO: get answer
	//TODO: create pipe
	//TODO: splice data from socket to pipe and pipe into file
    }
    return 0;
}
