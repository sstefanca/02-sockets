#include <stdio.h>

// hton, ntoh
#include <arpa/inet.h>

//socket, bind, listen
#include <sys/types.h>
#include <sys/socket.h>

#define QUEUE_LIMIT 50
#define BUF_SIZE 128

int backfd;
int count;

int main(int argc, char **argv)
{
    int sockfd;
    int port, ret;
    struct sockaddr_in sockaddr;
    printf("This is the server\n");
    if(argc != 2)
    {
	fprintf(stderr, "Usage: %s <port>\n", argv[0]);
	return 1;
    }

    ret = sscanf(argv[1], "%i", &port);
    if(ret != 1 || port > 0xffff || port < 1024)
    {
	fprintf(stderr, "Usage: Port number must be between %i and %i.\n", 0xffff, 1024);
	return 2;
    }

    printf("%i\n", port);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);
    sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    ret = bind(sockfd, (struct sockaddr *)&sockaddr, sizeof(sockaddr));
    if(ret != 0)
    {
	fprintf(stderr, "Error biniding port\n");
	return 3;
    }

    ret = listen(sockfd, QUEUE_LIMIT);
    if(ret != 0)
    {
	fprintf(stderr, "Error listening on port\n");
	return 4;
    }

    backfd = accept(sockfd, NULL, NULL);
    
    while(1)
    {
	char buf[BUF_SIZE];
	recv(backfd, buf, BUF_SIZE, 0);

	printf("%s", buf);
	++count;
	sprintf(buf, "backfd: %i %i", backfd, count);
	
	send(backfd, buf, BUF_SIZE, 0);
	//TODO: open file
	//TODO: create pipe pipe();
	//TODO: splice file into pipe and pipe into socket
    }
    return 0;
}
