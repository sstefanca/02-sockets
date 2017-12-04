#ifndef __HEADER__H__
#define __HEADER__H__ 1

#define REQUEST 1
#define ACK 2
#define NACK 3
#define FILE_NOT_FOUND 4

typedef struct header
{
    int msg;
    char path[128];
} header_t;

#endif
