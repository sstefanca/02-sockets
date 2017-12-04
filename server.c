#include <stdio.h>

// hton, ntoh
#include <arpa/inet.h>

//socket, bind, listen, open
#include <sys/types.h>
#include <sys/socket.h>

// epoll
#include <sys/epoll.h>

//open
#include <sys/stat.h>
#include <fcntl.h>

//close
#include <unistd.h>

//errno
#include <errno.h>

#include "header.h"

#define QUEUE_LIMIT 50
#define BUF_SIZE 0x10000
#define TIMEOUT 5 * 1000

int fds[200];

int main(int argc, char **argv)
{
    int sockfd, epollfd;
    int port, ret;
    struct sockaddr_in sockaddr;
    struct epoll_event event;
    printf("This is the server\n");

    //parsing arguments
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

    //setting up epoll
    epollfd = epoll_create(16);
    
    //opening socket for listening
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

    //adding socket to epoll
    event.data.fd = sockfd;
    event.events = EPOLLIN;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &event);

    //main loop
    while(1)
    {
	struct epoll_event ev;
	int sfd, ffd, num;

	//wait for event
	num = epoll_wait(epollfd, &event, 1, TIMEOUT);

	if(num == 0)
	    continue;
	//if socket is listen socket
	if(event.data.fd == sockfd)
	{

	    sfd = accept(sockfd, NULL, NULL);
	    ev.data.fd = sfd;
	    ev.events = EPOLLIN;

	    epoll_ctl(epollfd, EPOLL_CTL_ADD, sfd, &ev);
	}
	//if socket is client socket
	else
	{
	    sfd = event.data.fd;
	    //data avail to read?
	    if(event.events & EPOLLIN)
	    {
		header_t msg;

		//read data from socket
		recv(sfd, &msg, sizeof(msg), 0);
		if(msg.msg == REQUEST)
		{
		    ffd = open(msg.path, O_RDONLY);
		    //file opened correctly
		    if(ffd>0)
		    {
			fds[sfd]=ffd;
			msg.msg = ACK;
			send(sfd, &msg, sizeof(msg), 0);

			ev.data.fd = sfd;
			ev.events = EPOLLOUT;
			epoll_ctl(epollfd, EPOLL_CTL_MOD, sfd, &ev);
		    }
		    //file did not open correctly, whatever the reason
		    else
		    {
			msg.msg = FILE_NOT_FOUND;
			send(sfd, &msg, sizeof(msg), 0);
			close(sfd);
			
			epoll_ctl(epollfd, EPOLL_CTL_DEL, sfd, NULL);
		    }
		}
		//wrong message type
		else
		{
		    fprintf(stderr, "Wrong msg type received\n");
		    close(sfd);
		    epoll_ctl(epollfd, EPOLL_CTL_DEL, sfd, NULL);
		}
	    }
	    //space to write data?
	    else if(event.events & EPOLLOUT)
	    {
		char buf[BUF_SIZE];
		int len, ret;

		len = read(fds[sfd], buf, BUF_SIZE);
		if(len>0)
		{
		    ret = send(sfd, buf, len, MSG_DONTWAIT);
		    //do we have an error
		    if(ret<0)
		    {
			//is the error from MSG_DONTWAIT?
			if(errno != EAGAIN && errno != EWOULDBLOCK)
			{
			    fprintf(stderr, "Error sending on socket");
			    close(fds[sfd]);
			    fds[sfd]=0;
			    close(sfd);
			    epoll_ctl(epollfd, EPOLL_CTL_DEL, sfd, NULL);
			}
		    }
		}
		else if(len==0)
		{
		    close(fds[sfd]);
		    fds[sfd]=0;
		    close(sfd);
		    epoll_ctl(epollfd, EPOLL_CTL_DEL, sfd, NULL);
		}
	    }
	    //weird event, close socket
	    else
	    {
		if(fds[sfd]!=0)
		    close(fds[sfd]);
		close(sfd);
		epoll_ctl(epollfd, EPOLL_CTL_DEL, sfd, NULL);
	    }
	}
    }
    return 0;
}
