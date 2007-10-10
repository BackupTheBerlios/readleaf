/*
 * RedLeaf IPC
 *
 * Copyright (C) 2006, 2007 RedLeaf devteam org.
 *
 * Written by Tirra (tirra.newly@gmail.com)
 *  
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */ 


#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

#include <http.h>

#define PORT  8080
#define MAX_MSG  4096

#define _DEBUG_  1

void i_saddr(struct sockaddr_in *v,const char *host,uint16_t port)
{
  struct hostent *h=NULL;

  v->sin_family=AF_INET;
  v->sin_port=htons(port);
  h=gethostbyname(host);
  if(!h)
    v->sin_addr.s_addr=htonl(INADDR_ANY);
  else
    v->sin_addr=*(struct in_addr *) h->h_addr;

  return;
}

int cr_sock(uint16_t p,const char *host)
{
  int sk;
  struct sockaddr_in s;

  sk=socket(PF_INET,SOCK_STREAM,0);
  if(sk<0) {
    fprintf(stderr,"Error createting socket.\n");
    return -1;
  }
  i_saddr(&s,host,80);
  if(bind(sk,(struct sockaddr *)&s,sizeof(s))<0) {
    perror("bind");
    fprintf(stderr,"Error on binding socket.\n");
    return -1;
  }
  return sk;
}

int get_cl_data(int fd)
{
  char buf[MAX_MSG];
  int n;
  struct http_request *o;

  memset(buf,'\0',MAX_MSG);
  n=read(fd,buf,MAX_MSG);
  if(n<0) {
    perror("read:");
  } else if(n==0)
    return -1;
  else {
#ifdef _DEBUG_
    fprintf(stdout,"SRV->MSG: \"%s\"\n",buf);
#endif
    o=parse_http_request(buf);
    /*process the request*/
    process_request(o,fd);
    free_http_request(o);
    return 0;
  }
  return 0;
}

int main_process(int argc,char **argv) 
{
  fd_set active_fd_set,read_fd_set;
  struct sockaddr_in claddr;
  int i,sock=cr_sock(PORT,"localhost");
  int n;
  socklen_t s=sizeof(struct sockaddr_in);

  if(sock==-1)
    exit(3);
  if(listen(sock,1)<0) {
    fprintf(stderr,"Error on socket listening.\n");
    exit(3);
  }

  FD_ZERO(&active_fd_set);
  FD_SET(sock,&active_fd_set);

  while(1) {
    read_fd_set=active_fd_set;
    if(select(FD_SETSIZE,&read_fd_set,NULL,NULL,NULL)<0){
      perror("select");
      exit(3);
    }
    for(i=0;i<FD_SETSIZE;i++) {
      if(FD_ISSET(i,&read_fd_set)) {
	if(i==sock) {
	  n=accept(sock,(struct sockaddr *)&claddr,&s);
	  if(n<0) {
	    perror("accept:");
	    exit(3);
	  }
	  //	  fprintf(stdout,"Connected from %s(%d).\n",inet_ntoa(claddr.sin_addr,claddr.sin_port));
	  FD_SET(n,&active_fd_set);
	} else { /*continuing*/
	  if(get_cl_data(i)<0) {
	    close(i);
	    FD_CLR(i, &active_fd_set);
	  }
	}
      }
    }

    //    usleep(100);
  }


  return 0;
}
