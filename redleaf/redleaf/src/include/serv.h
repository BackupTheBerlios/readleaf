
#ifndef __SERV_H__
#define __SERV_H__

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>


void i_saddr(struct sockaddr_in *v,const char *host,uint16_t port);
int cr_sock(uint16_t p,const char *host);
int get_cl_data(int fd);
int main_process(int argc,char **argv);


#endif
