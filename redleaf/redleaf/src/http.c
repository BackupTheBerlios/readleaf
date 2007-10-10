/*
 * RedLeaf HTTP 1.1 implementation
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

/*http 1.1 implementation*/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include <http.h>

#define _DEBUG_  1

/*functions prototypes*/
void free_http_reply(struct http_reply *reply);
static void init_http_request(struct http_request *p);

/*TODO: exchange malloc/free/strdup/strndup to safe internal functions*/
/*TODO: parse other variables from request*/
struct http_request *parse_http_request(char *msg)
{
  struct http_request *p=NULL;
  char *tmsg=msg,*ttmsg;

  if(!msg) {
    fprintf(stderr,"Request is NULL\n");
    return NULL;
  }
  if(!(p=malloc(sizeof(struct http_request)))) {
    fprintf(stderr,"Enough memory for allocate the structure.\n");
    return NULL;
  }
  if(!strncmp(msg,"GET",3))
    p->method=GET;
  else if(!strncmp(msg,"POST",4))
    p->method=POST;
  else {
  bad_request:
    p->op_code=BAD_REQUEST;
    return p;
  }
  if((tmsg=strchr(msg,' ')) && (ttmsg=strchr(tmsg+1,' '))){
    printf("ttmsg '%s'\n",ttmsg);
    *ttmsg='\0';
    p->uri=strdup(tmsg+sizeof(char));
    *ttmsg=' ';
  } else goto bad_request;
  if(strncmp(ttmsg+sizeof(char),"HTTP/1.1",8)) 
    goto bad_request;

  return p;
}

char *print_rhead(int op_code)
{
  char *o=NULL;
  int hl=strlen((const char *)"HTTP/1.1 200 OK\n")+1;
  if(!op_code)
    return NULL;
  o=malloc(hl);
  if(!o)
    return NULL;
  snprintf(o,hl,"HTTP/1.1 200 OK\n");
  return o;
}

struct http_reply *generate_reply(int operation_code)
{
  char *date=NULL;
  struct http_reply *reply=malloc(sizeof(struct http_reply));


  /*TODO: normal checking and filling*/
  reply->head=strdup("HTTP/1.1 200 OK");
  reply->server=strdup("Server: RedLeaf v0.1a");
  reply->connection_type=strdup("Connection: closed");
  date=get_rfc1123date(time(NULL));
  reply->fmtdate=malloc(32+strlen("Date: "));
  sprintf(reply->fmtdate,"Date: %s",date);
  free(date);
  
  return reply;
}

int write_http_reply(struct http_reply *r,int fd)
{
  char buf[512];
  int wl=0;

  memset(buf,'\0',512);
  sprintf(buf,"%s\n%s\n%s\nContent-Length: %d\n%s\n%s\n\n",r->head,r->fmtdate,r->server,
	  r->content_length,r->connection_type,r->content_type);
  wl=write(fd,buf,strlen(buf));
  wl+=write(fd,r->buf,r->content_length);

  return 0;
}

/*TODO: add normal processing*/
int process_request(struct http_request *r,int fd)
{
  struct http_reply *reply=generate_reply(200);
  char *req=r->uri; *req++;
  int i=0;
  void *out;

  printf("r: %s\n",req);
  out=mmap_file("/home/kaanoken/works/redleaf/cvs/redleaf/redleaf/src/404.html",&i);
  if(!i)
    return 0;
  printf("len: %d\n",i);
  reply->content_length=i;
  reply->content_type=strdup("Content-Type: text/html");
  reply->buf=out;

  write_http_reply(reply,fd);

  munmap(out,i);
  free_http_reply(reply);

  return 0;
}

void free_http_reply(struct http_reply *reply)
{
  if(!reply)
    return;
  if(reply->head)
    free(reply->head);
  if(reply->fmtdate)
    free(reply->fmtdate);
  if(reply->server)
    free(reply->server);
  if(reply->content_type)
    free(reply->content_type);
  if(reply->connection_type)
    free(reply->connection_type);
  free(reply);

  return;
}

/*TODO: make full freeing*/
void free_http_request(struct http_request *p)
{
  if(!p)
    return;
  if(p->uri)    free(p->uri);
  if(p->host)    free(p->host);
  if(p->user_agent) free(p->user_agent);
  if(p->accept) free(p->accept);
  if(p->accept_language) free(p->accept_language);
  if(p->accept_encoding) free(p->accept_encoding);
  if(p->accept_charset) free(p->accept_charset);
  free(p);

  return;
}

static void init_http_request(struct http_request *p)
{
  if(!p)
    return;
  p->uri=p->host=p->user_agent=p->accept=p->accept_language=p->accept_encoding=NULL;
  p->accept_charset=NULL;

  return;
}
