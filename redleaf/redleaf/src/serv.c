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
#include <sys/wait.h>
#include <sys/mman.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

#include "../config.h"

#include <http.h>
#include <file.h>
#include <serv.h>
#include <misc.h>
#include <conf.h>
#ifdef MODULAS
#include <modula.h>
#endif

#define PORT             8080  /*port to listen*/
#define MAX_MSG          4096  /*max message length (from client)*/
#define MAX_CONNECTIONS  10    /*max number of client per time*/

#define _DEBUG_  0 /*debug info macro*/
#undef _DEBUG_

/*
 * This structure used for balancing purposes
 * if we will have many childs at one time, we can 
 * expect many time loss in the case of the kernel
 * checking.
 */
struct load_chld_t {
  int st; /*state info, also reserved for flags*/
  unsigned long curr_conn; /*current connections on child*/
  unsigned long total_conn; /*total connections passed to child*/
};

/*local varios variables*/
static struct connection_t **connections=NULL;
static int total_connections=0;
static int max_connections=0;
static int sock=-1;
static fd_set fdrdset,fdwrset;
static time_t current_time;
static pid_t *chlds=NULL;
static struct load_chld_t *load_chld; /*Shared for all childs*/

/*local functions prototypes*/
static int _serv_proc(int sock,int max_conn,int id);
static int __make_chld(int sock,int max_conn,int id);
static void __sigint_handler(int sig_num);
static void sigint_handler(int signal_number);
/*static void sigchld_handler(int signal_number);*/
static void sigpipe_handler(int signal_number);
static void init_connections(int max);
static int new_connection(void);
static void read_connection(int i);
static void parse_connection(int i);
static void write_connection(int i);
static void close_connection(int i);
static void i_saddr(struct sockaddr_in *v,const char *host,uint16_t port);
static int cr_sock(uint16_t p,const char *host);
static int shutdown_socket(void);
/*static int unblock_socket(int sk);*/
/*debug output functions*/
#ifdef _DEBUG_ 
static void dbg_print_connection_info(int i);
#endif

int main_process(int argc,char **argv) 
{
  char *cnf_value=get_general_value("port");
  int port_n=8080,i;

  if(cnf_value) 
    port_n=atoi(cnf_value);

  cnf_value=get_general_value("hostname");

  sock=cr_sock(port_n,(cnf_value==NULL) ? "localhost" : cnf_value);
  if(sock==-1)
    exit(3);
  if(listen(sock,128)<0) {
    fprintf(stderr,"Error on socket listening.\n");
    shutdown_socket();
    exit(3);
  }

  cnf_value=get_general_value("max_clients");

  port_n=(cnf_value==NULL) ? MAX_CONNECTIONS : atoi(cnf_value);
  max_connections=(port_n<2) ? 2 : port_n;

  chlds=rl_malloc(max_connections*sizeof(pid_t));
  if(!chlds) {
    fprintf(stderr,"Error allocating memory.\n");
    shutdown_socket();
    exit(-1);
  }

  /*init structure used for balancing*/
  load_chld=mmap(0,sizeof(struct load_chld_t)*(max_connections/2),PROT_READ | PROT_WRITE,
		 MAP_ANON | MAP_SHARED,-1,0);
  for(i=0;i<=max_connections/2;i++) {
    load_chld[i].st=1;
    load_chld[i].curr_conn=0;
    load_chld[i].total_conn=0;
  }

  for(i=0;i<=max_connections/2;i++)
    chlds[i]=__make_chld(sock,max_connections,i);

  /*add signal handling*/
  signal(SIGINT,__sigint_handler);
  signal(SIGPIPE,sigpipe_handler);

  for(;;) {
#ifdef _DEBUG_
    for(i=0;i<=max_connections/2;i++) {
      fprintf(stderr,"Child<%d> info::\n\tState: %d\n",i,load_chld[i].st);
      fprintf(stderr,"\tCurrent connections: %ld\n",load_chld[i].curr_conn);
      fprintf(stderr,"\tTotal passed connections: %ld\n",load_chld[i].total_conn);
    }

    sleep(10);
#endif
    pause();
  }

  munmap(load_chld,sizeof(struct load_chld_t)*(max_connections/2));
  
  return 0;
}

/*================implementation==============================*/

static int __make_chld(int sock,int max_conn,int id)
{
  pid_t pid;

  if((pid=fork())>0)
    return pid;

  signal(SIGINT,sigint_handler);
  signal(SIGPIPE,sigpipe_handler);

  _serv_proc(sock,max_conn,id);

  return 0;
}

static int _serv_proc(int sock,int max_conn,int id)
{
  fd_set trdset,twrset;
  struct timeval tm;
  int i;

  init_connections(max_conn);

  current_time=time(NULL);

  FD_ZERO(&fdrdset);
  FD_ZERO(&fdwrset);
  FD_SET(sock,&fdrdset);

  while(1) {
    FD_SET(sock,&fdrdset);
    current_time=time(NULL);
    trdset=fdrdset;
    twrset=fdwrset;

    tm.tv_sec=0;
    tm.tv_usec=max_connections;

    if(select(FD_SETSIZE,&trdset,NULL,NULL,&tm)<0){
      perror("select");
      shutdown_socket();
      exit(3);
    }

    if(FD_ISSET(sock,&trdset)) {/*try to create new connection*/
      load_chld[id].curr_conn++;
      load_chld[id].total_conn++;
      new_connection();
    }
    
    for(i=0;i<max_connections;i++) {
      if(connections[i]==NULL)
	break;
      if(connections[i]->socket==-1)
	continue;

      if(FD_ISSET(connections[i]->socket,&trdset)) { /*selected to read*/
	connections[i]->last_state=current_time;
	if(connections[i]->rxstat==ST_NONE)	  connections[i]->rxstat=ST_PRCS;
	read_connection(i); /*try to read in general case*/

#if 0
	if(connections[i]->rxstat==ST_PRCS) /*parse if necessary*/
	  parse_connection(i); 
#endif
      }

      if(connections[i]->rxstat==ST_PRCS && /*catch timeout while read*/
	 current_time - connections[i]->last_state > RD_TIMEOUT)
	connections[i]->rxstat=ST_TIMEOUT;
      
      /*writing connections*/
      if(/*FD_ISSET(connections[i]->socket,&twrset)*/connections[i]->rxstat==ST_DONE ) {
	connections[i]->last_state=current_time;
	write_connection(i); 
      }

      /*check the states for close droped connections*/
      if((connections[i]->rxstat==ST_ERROR || 
	  connections[i]->rxstat==ST_TIMEOUT ||
	  connections[i]->rxstat==ST_DONE) &&
	 (connections[i]->wxstat==ST_ERROR ||
	  connections[i]->wxstat==ST_DONE)){
	load_chld[id].curr_conn--;
	close_connection(i); 
      }
    }
    
  }

  return 0;
}

static void close_connection(int i)
{
  FD_CLR(connections[i]->socket,&fdwrset);
  FD_CLR(connections[i]->socket,&fdrdset);
  total_connections--;
  close(connections[i]->socket);
  connections[i]->socket=-1;
  connections[i]->last_state=current_time;
  connections[i]->data_len=0;
  if(connections[i]->page!=NULL)
    connections[i]->page->ref--;
#ifdef MODULAS
  if(connections[i]->page->op==4) {
    connections[i]->page->emod->modula_session_close(connections[i]->page->emod);
    //free_page_t(connections[i]->page);
  }
#endif
  connections[i]->page=NULL;
  connections[i]->data_ptr=connections[i]->data=NULL;
  connections[i]->data_send=NULL;
  if(connections[i]->file)
    destroy_file_session(connections[i]->file);
  connections[i]->file=NULL;
  connections[i]->request_len=0;
  connections[i]->http=http_session_close(connections[i]->http);

  return;
}

static void write_connection(int i)
{
  int res;
  struct page_t *page;
  void *buf=NULL;
  size_t flen;
  off_t old_off;

  if(connections[i]->wxstat==ST_ERROR || connections[i]->wxstat==ST_DONE)
    return;

  if(connections[i]->page==NULL) {
    connections[i]->rxstat=connections[i]->wxstat=ST_ERROR;
    return;
  } else
    page=connections[i]->page;

  connections[i]->wxstat=ST_PRCS;

  if(connections[i]->hb_switch==SS_HEAD) { /*first time to send*/
    connections[i]->data=connections[i]->data_ptr=page->head;
    connections[i]->data_len=page->bodysize+page->head_len;
    connections[i]->hb_switch=SS_BODY;
  } else if(connections[i]->data_len==0 && 
	    connections[i]->hb_switch!=SS_HEAD) { /*data sent*/
    switch(page->op) {
    case 2:
      connections[i]->hb_switch=SS_FILE;
      break;
#ifdef MODULAS
    case 4:
      connections[i]->hb_switch=SS_MODULA;
      break;
#endif
    default:
      connections[i]->wxstat=ST_DONE;
      FD_CLR(connections[i]->socket,&fdrdset);
      return;
      break;
    }
  }
  if(connections[i]->hb_switch==SS_BODY) {
    res=write(connections[i]->socket,connections[i]->data_ptr,
	      connections[i]->data_len);
    if(res<0 && errno!=EPIPE)
      return;
    if(res<0) {
      fprintf(stderr,"Error on writing to the socket.\n");
      connections[i]->wxstat=ST_ERROR;
      FD_CLR(connections[i]->socket,&fdrdset);
      return;
    }
    connections[i]->data_len-=res;
    connections[i]->data_ptr+=res;
  }
  if(connections[i]->hb_switch==SS_FILE) { /*sending*/
    if(connections[i]->data_len==0) { /*set parameters*/
      connections[i]->data_len=connections[i]->file->file_len;	  
      if(page->range) {
	connections[i]->data_len-=page->range;
	updoffset_file_session(connections[i]->file,page->range);
      }
    }
    old_off=connections[i]->file->cur_off;
    buf=read_file_session(connections[i]->file,&flen);
    res=write(connections[i]->socket,buf,flen);
    if(res<0 && errno!=EPIPE) {
      updoffset_file_session(connections[i]->file,old_off);
      return;
    } 
    if(res<0) {
      fprintf(stderr,"Error on writing to the socket.\n");
      perror("write:");
      connections[i]->wxstat=ST_ERROR;
      FD_CLR(connections[i]->socket,&fdrdset);
      return;      
    }
    connections[i]->data_len-=res;
    updoffset_file_session(connections[i]->file,old_off+res);
  }
#ifdef MODULAS
  if(connections[i]->hb_switch==SS_MODULA) { /*heh, transfer modula's output*/
    char rd[16]; char *dta;
    modula_session_t *ses=connections[i]->page->emod;
    size_t rres=0; int cc;

    res=ses->modula_session_read(ses,&rd,sizeof(char)*16);
    if(res>0) {
      dta=rd;       cc=res;
      while(cc!=rres) { /*while our 16 bytes will not written try to send it*/
	rres=write(connections[i]->socket,dta,cc);
	if(rres<0 && errno!=EPIPE)
	  return;
	if(rres<0) {
	  fprintf(stderr,"Error on writing to the socket.\n");
	  connections[i]->wxstat=ST_ERROR;
	  FD_CLR(connections[i]->socket,&fdrdset);
	  return;
	}
	cc-=rres; dta+=rres;
      }
    }
    else {
#if 0
      ses->modula_session_close(ses);
#endif
      connections[i]->hb_switch=SS_DONE;
    }
    /*rich out, space*/
  }
#endif
  if(connections[i]->hb_switch==SS_FILE && connections[i]->data_len==0) 
    connections[i]->hb_switch=SS_DONE;
  if(connections[i]->hb_switch==SS_DONE) { /*uff, all sent*/
    if(page->op==2)
      destroy_file_session(connections[i]->file);
#ifdef MODULAS
    //    if(page->op==4)
    //page->emod->modula_session_close(page->emod);
#endif
    connections[i]->file=NULL;
    connections[i]->wxstat=ST_DONE;
    FD_CLR(connections[i]->socket,&fdrdset);
  }
    
  return;
}
#if 0
static void parse_connection(int i) /*simply request the page*/
{
  struct page_t *page;

  printf("-----\n%s\n------\n",connections[i]->request);
  connections[i]->page=page_t_generate(connections[i]->request);
#ifdef _DEBUG_
  char tnm[50];
  snprintf(tnm,sizeof(tnm)-1,"http-%d.txt",getpid());
  printf("save http reuest to: %s",tnm);
  FILE *f=fopen(tnm,"w");
  if(f){
	fwrite(connections[i]->request,connections[i]->request_len,1,f);
	fclose(f);
  }
#endif
  if(connections[i]->page==NULL)
    connections[i]->rxstat=connections[i]->wxstat=ST_ERROR;

  page=connections[i]->page;
  if(page->op==2) 
    connections[i]->file=create_file_session(page->filename,4096);
#ifdef MODULAS
  if(page->op==4 && !page->emod)
    fprintf(stderr,"Oops!\n");
#endif

  page->ref++;

  connections[i]->rxstat=ST_DONE;
  FD_CLR(connections[i]->socket,&fdrdset);

  return;
}
#endif
#ifdef _DEBUG_

static void dbg_print_connection_info(int i)
{
  char *buf=rl_malloc(64);
  char *date=NULL;
  fprintf(stderr,"connection[%d] stat:\n",i);
  switch(connections[i]->rxstat) {
  case ST_READ:
    buf=strdup("READ");
    break;
  case ST_PRCS:
    buf=strdup("PROCESS");
    break;
  case ST_NONE:
    buf=strdup("NONE");
    break;
  case ST_TIMEOUT:
    buf=strdup("TIMEOUTED");
    break;
  case ST_ERROR:
    buf=strdup("ERROR");
    break;
  case ST_DONE:
    buf=strdup("DONE");
    break;
  }
  fprintf(stderr,"\tRXSTAT: %s",buf);
  switch(connections[i]->wxstat) {
  case ST_READ:
    buf=strdup("READ");
    break;
  case ST_PRCS:
    buf=strdup("PROCESS");
    break;
  case ST_NONE:
    buf=strdup("NONE");
    break;
  case ST_TIMEOUT:
    buf=strdup("TIMEOUTED");
    break;
  case ST_ERROR:
    buf=strdup("ERROR");
    break;
  case ST_DONE:
    buf=strdup("DONE");
    break;
  }
  fprintf(stderr," WXSTAT: %s\n",buf);
  date=get_rfc1123date(connections[i]->last_state);
  fprintf(stderr,"\tLast processed time: %s\n",date);
  fprintf(stderr,"\tRequest length: %d\n",connections[i]->request_len);
  fprintf(stderr,"\tRequest: \"%s\"\n",connections[i]->request);

  if(connections[i]->page)
    fprintf(stderr,"\tPage URI: %s\n",connections[i]->page->uri);
  else
    fprintf(stderr,"\tPage is NULL\n");

  rl_free(date);
  rl_free(buf);

  return;
}

#endif /*_DEBUG_*/

static void read_connection(int i)
{
  int rd;
  char buf[32];

  if(connections[i]->rxstat==ST_ERROR || connections[i]->rxstat==ST_TIMEOUT ||
     connections[i]->rxstat==ST_DONE)
    return;

  memset(buf,'\0',32);
  rd=read(connections[i]->socket,buf,31);

  if(rd<0 && errno==EWOULDBLOCK)    return;
  if(rd<0) {
    connections[i]->rxstat=connections[i]->wxstat=ST_ERROR; /*error on read*/
    FD_CLR(connections[i]->socket,&fdrdset);
    return;
  }

  switch(http_session_process(connections[i]->http,buf,rd)) {
  case CX_NO:
    return;
    break;
  case CX_PAGE_READY:
    connections[i]->rxstat=ST_DONE;
    FD_CLR(connections[i]->socket,&fdrdset);
    connections[i]->page=http_session_gen_page(connections[i]->http);
    //    fprintf(stderr,"page->filename=%s\n",connections[i]->page->filename);
    break;
  case CX_PAGE_READ_BODY:
    /*read to modula*/
    if(!connections[i]->page) /*first time to generate*/
      connections[i]->page=http_session_gen_page(connections[i]->http);
#ifdef MODULAS
    if(!rd) {
      connections[i]->rxstat=ST_DONE;
      FD_CLR(connections[i]->socket,&fdrdset);
    }
    /*TODO: read to modula if possible*/
#else
    connections[i]->rxstat=ST_DONE;
    FD_CLR(connections[i]->socket,&fdrdset);
#endif

    break;
  }

#ifdef _DEBUG_
  dbg_print_connection_info(i);
#endif

  return;
}

static int new_connection(void)
{
  int i,cl;
  socklen_t rin_len;
  int o=1;
  struct sockaddr_in rin;

  rin_len=sizeof(rin);

  total_connections++;
  for(i=0;i<max_connections;i++) 
    if(connections[i]==NULL || connections[i]->socket==-1)
      break;
  if(i==max_connections) { /*somebody will be kicked off*/
    cl=accept(sock,(struct sockaddr *) &rin,&rin_len);
    fprintf(stderr,"Too many connections. Dropping connection.\n");
    close(cl);
    return -1;
  } else {
    if(!connections[i]) {
      connections[i]=rl_calloc(1,sizeof(struct connection_t));
      connections[i]->socket=-1;
      connections[i]->request=rl_malloc(MAXBUF_LEN);
    }
    connections[i]->last_state=current_time;
    connections[i]->rxstat=connections[i]->wxstat=ST_NONE;
    connections[i]->hb_switch=SS_HEAD;
    connections[i]->file=NULL;
    connections[i]->page=NULL;
    connections[i]->req_ptr=connections[i]->request;
    connections[i]->request_len=0;
    memset(connections[i]->request,'\0',MAXBUF_LEN);
    connections[i]->socket=accept(sock,(struct sockaddr *) &rin,&rin_len);

    if(connections[i]->socket<0) {
      fprintf(stderr,"Accept failed.\n");
      perror("accept() :");
      connections[i]->socket=-1;
      return -1;
    } else {
      if(fcntl(connections[i]->socket,F_SETFL,
	       fcntl(connections[i]->socket,F_SETFL,0) | FNDELAY) < 0) {
	fprintf(stderr,"fcntl failed.\n");
	close(connections[i]->socket);
	connections[i]->socket=-1;
	return -1;
      } else {
	if(setsockopt(connections[i]->socket,SOL_SOCKET,SO_REUSEADDR,
		      (char *) &o, sizeof(int))==-1)
	  fprintf(stderr,"setsockopt() failed.\n");
	FD_SET(connections[i]->socket,&fdrdset); /*connected*/
	connections[i]->addr=rin.sin_addr;
	connections[i]->http=rl_malloc(sizeof(http_session_t));
	connections[i]->http=http_session_open(connections[i]->http,connections[i]->addr.s_addr);
      }
    }
  }

  fprintf(stderr,"new_connection(%d)->socket=%d\n",i,connections[i]->socket);    

  return connections[i]->socket;
}

static void sigint_handler(int signal_number)
{
  fprintf(stdout,"Shutting down daemon ... \n");
  fflush(stdout);
  shutdown_socket();
  exit(0);

  return;
}

static void __sigint_handler(int sig_num)
{
  int i;
 
  for(i=0;i<=max_connections/2;i++) 
    kill(chlds[i],SIGINT);
  while(wait(NULL)>0) ;

  munmap(load_chld,sizeof(struct load_chld_t)*(max_connections/2));

  exit(0);

  return;
}

#if 0
static void sigchld_handler(int signal_number)
{
  while (waitpid(-1,&signal_number,WNOHANG) > 0) ;
}
#endif

static void sigpipe_handler(int signal_number)
{
  /*dummy*/
}

static void init_connections(int max)
{
  connections=calloc(max,sizeof(struct connection_t));
  if(!connections) {
    fprintf(stderr,"Cannot allocate memory for connections.\nExiting.\n");
    exit(3);
  }

  max_connections=max;
  total_connections=0;

  return;
}

static void i_saddr(struct sockaddr_in *v,const char *host,uint16_t port)
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

static int cr_sock(uint16_t p,const char *host)
{
  int sk,n=1;
  struct sockaddr_in s;

  sk=socket(PF_INET,SOCK_STREAM,0);
  if(sk<0) {
    fprintf(stderr,"Error createting socket.\n");
    return -1;
  }

  setsockopt(sk,SOL_SOCKET,SO_REUSEADDR,&n,sizeof(n));

  i_saddr(&s,host,p);
  if(bind(sk,(struct sockaddr *)&s,sizeof(s))<0) {
    perror("bind");
    fprintf(stderr,"Error on binding socket.\n");
    return -1;
  }

  return sk;
}

#if 0
static int unblock_socket(int sk)
{
  int flags=fcntl(sk,F_GETFL,NULL);
  if (flags==-1)
    perror("fcntl(): ");
  flags|=O_NONBLOCK;
  if(fcntl(sk, F_SETFL, flags) == -1)
    perror("fcntl(): ");

  return 0;
}
#endif

static int shutdown_socket(void)
{
  if(shutdown(sock,SHUT_RDWR)==-1){
    perror("shutdown:");
    return -1;
  }
  return 0;
}

