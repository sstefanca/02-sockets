#include <stdio.h>
#include <stdlib.h>

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
#define BUF_SIZE 2048

void socket_to_file(int sock, int file)
{
    int retval, len, count;
    char buf[BUF_SIZE];

    retval = recv(sock, buf, BUF_SIZE, 0);
    while(retval>0)
    {
	count = retval;
	len = 0;
	while(count>0)
	{
	    retval = write(file, buf + len, count);
	    if(retval <= 0)
	    {
		fprintf(stderr, "Error writing to file\n");
		exit(1);
	    }

	    len += retval;
	    count -= retval;
	}
	retval = recv(sock, buf, BUF_SIZE, 0);
    }
}

int main(int argc, char **argv)
{
    int sockfd;
    int port, ret;
    struct sockaddr_in sockaddr;
    printf("This is the client\n");

    //parsing arguments
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

    //setting up socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);
    sockaddr.sin_addr.s_addr = inet_addr(argv[1]);

    //connecting to server
    ret = connect(sockfd, (struct sockaddr *)&sockaddr, sizeof(sockaddr));
    if(ret != 0)
    {
	fprintf(stderr, "Error connecting to server!\n");
	return 3;
    }

    //sending request and receiving message
    {
	header_t head;
	int fd;

	head.msg = REQUEST;
	strncpy(head.path, argv[3], PATH_LENGTH - 1);

	send(sockfd, &head, sizeof(head), 0);
	recv(sockfd, &head, sizeof(head), 0);

	switch(head.msg)
	{
	case ACK:
	    fd = open(argv[3], O_CREAT | O_WRONLY | O_TRUNC);
	    socket_to_file(sockfd, fd);

	    close(fd);
	    close(sockfd);
	    break;

	case NACK:
	case FILE_NOT_FOUND:
	    printf("Wrong id received %i\n", head.msg);
	    break;
	}
    }
    return 0;
}
