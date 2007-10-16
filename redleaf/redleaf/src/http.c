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
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>

#include <http.h>
#include <http_read_dir.h>
#include <misc.h>

#define _DEBUG_  1

/*functions prototypes*/
void free_http_reply(struct http_reply *reply);
static void init_http_request(struct http_request *p);
static char *mime_type(char *path);
static char *read_directory_entry(DIR *dir);
static void free_ppchar(char **n,int size);
static int qsort_cmp(const void *a, const void *b);

/*TODO: exchange malloc/free/strdup/strndup to safe internal functions*/
/*TODO: parse other variables from request*/
struct http_request *parse_http_request(char *msg)
{
  struct http_request *p=NULL;
  char *tmsg=msg,*ttmsg;

  if(!msg || strlen(msg)<6) {
    fprintf(stderr,"Request is NULL or empty\n");
    return NULL;
  }
  if(!(p=malloc(sizeof(struct http_request)))) {
    fprintf(stderr,"Enough memory for allocate the structure.\n");
    return NULL;
  }
  init_http_request(p);
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
  char buf[512],cc;
  char html[1024];
  int wl=0,mode=0;
  int cd=-1,opcode=OK;
  struct stat ystat;

  /*check the file is exist*/
  cd=open(r->buf,O_RDONLY);
  if(cd==-1){/*TODO: look for rfc if is it right*/
    perror("open: ");
    free(r->head);
    if(errno==EACCES){
      r->head=strdup("HTTP/1.1 403 Forbidden");
      opcode=FORBIDDEN;
    } else { 
      r->head=strdup("HTTP/1.1 404 Not Found");
      opcode=NOT_FOUND;
    }
    mode=3;
  } else {
    opcode=OK;
    fstat(cd,&ystat);
    if(S_ISREG(ystat.st_mode)){
      mode=0;
      r->content_length=ystat.st_size;
    } else if(S_ISDIR(ystat.st_mode))
      mode=1;
    else {
      free(r->head);
      r->head=strdup("HTTP/1.1 403 Forbidden");
      mode=3;
    }
  }
  memset(buf,'\0',512);

  if(mode==0){
    sprintf(buf,"%s\n%s\n%s\nContent-Length: %ld\n%s\n%s\n\n",r->head,r->fmtdate,r->server,
	    r->content_length,r->connection_type,r->content_type);
    wl=write(fd,buf,strlen(buf));
    /*TODO: better pipe->pipe transactions*/
    while(read(cd,&cc,sizeof(char)))
      wl+=write(fd,&cc,sizeof(char));
    close(cd);
  } else if(mode==1){
    sprintf(buf,"<html><head><title>%s:</title></head><body>",r->uri);
    wl+=write(fd,buf,strlen(buf));
    sprintf(buf,"<h1>Directory %s:</h1><hr>",r->uri);
    wl+=write(fd,buf,strlen(buf));
    wl+=write_html_dir_list(fd,r->buf,r->uri);
    sprintf(buf,"<hr>RedLeaf httpd v0.1a</body></html>");
    wl+=write(fd,buf,strlen(buf));
  } else {
    switch(opcode) {
    case FORBIDDEN:
      memset(html,'\0',1024);
      snprintf(html,1024,"<html><head><title>Forbidden</title></head> \
	       <body><h1>Forbidden</h1><hr>RedLeaf v0.1a</body></html>\n");
      r->content_length=strlen(html);
      sprintf(buf,"%s\n%s\n%s\nContent-Length: %ld\n%s\n%s\n\n",r->head,r->fmtdate,r->server,
	      r->content_length,r->connection_type,r->content_type);
      wl=write(fd,buf,strlen(buf));
      wl+=write(fd,html,strlen(html));
      break;
    case NOT_FOUND:
      memset(html,'\0',1024);
      snprintf(html,1024,"<html><head><title>Not Found</title></head>"
	       "<body><h1>%s not found on this server</h1><hr>RedLeaf v0.1a</body></html>\n",r->uri);
      r->content_length=strlen(html);
      sprintf(buf,"%s\n%s\n%s\nContent-Length: %ld\n%s\n%s\n\n",r->head,r->fmtdate,r->server,
	      r->content_length,r->connection_type,r->content_type);
      wl=write(fd,buf,strlen(buf));
      wl+=write(fd,html,strlen(html));
      break;
    }
    return -1;
  }

  return wl;
}

/*TODO: add normal processing*/
int process_request(struct http_request *r,int fd)
{
  struct http_reply *reply=generate_reply(r->op_code);
  char *req=r->uri;
  char *filepath=NULL;

  printf("r: %s\n",req);
  /*TODO: remake this for working with CM*/
  filepath=getcwd(filepath,0);
  filepath=realloc(filepath,strlen(filepath)+strlen(req)+sizeof(char));
  filepath=strcat(filepath,req);

  reply->content_length=0;
  reply->content_type=strdup(strdup(mime_type(filepath)));
  reply->buf=filepath;
  reply->uri=strdup(req);

  if(write_http_reply(reply,fd)==-1){
    free_http_reply(reply);
    return -1;
  }

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
  if(reply->uri)
    free(reply->uri);
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

static char *mime_type(char *path)
{
  char *d=NULL;
  d=strrchr(path,'.');
  if(!d)
    return "Content-Type: text/plain";
  else if(!strcasecmp(d,".html"))
    return "Content-Type: text/html";
  else if(!strcasecmp(d,".htm"))
    return "Content-Type: text/html";
  else if(!strcasecmp(d,".txt"))
    return "Content-Type: text/plain";
  else if(!strcasecmp(d,".text"))
    return "Content-Type: text/plain";
  else if(!strcasecmp(d,".aiff"))
    return "audio/x-aiff";
  else if(!strcasecmp(d,".au"))
    return "audio/x-au";
  else if(!strcasecmp(d,".gif"))
    return "image/gif";
  else if(!strcasecmp(d,".png"))
    return "image/png";
  else if(!strcasecmp(d,".bmp"))
    return "image/bmp";
  else if(!strcasecmp(d,".jpeg"))
    return "image/jpeg";
  else if(!strcasecmp(d,".jpg"))
    return "image/jpeg";
  else if(!strcasecmp(d,".tiff"))
    return "image/tiff";
  else if(!strcasecmp(d,".tif"))
    return "image/tiff";
  else if(!strcasecmp(d,".pnm"))
    return "image/x-portable-anymap";
  else if(!strcasecmp(d,".pbm"))
    return "image/x-portable-bitmap";
  else if(!strcasecmp(d,".pgm"))
    return "image/x-portable-graymap";
  else if(!strcasecmp(d,".ppm"))
    return "image/x-portable-pixmap";
  else if(!strcasecmp(d,".rgb"))
    return "image/rgb";
  else if(!strcasecmp(d,".xbm"))
    return "image/x-bitmap";
  else if(!strcasecmp(d,".xpm"))
    return "image/x-pixmap";
  else if(!strcasecmp(d,".mpeg"))
    return "video/mpeg";
  else if(!strcasecmp(d,".mpg"))
    return "video/mpeg";
  else if(!strcasecmp(d,".ps"))
    return "application/ps";
  else if(!strcasecmp(d,".eps"))
    return "application/ps";
  else if(!strcasecmp(d,".dvi"))
    return "application/x-dvi";
  return "text/plain";
}

static char *read_directory_entry(DIR *dir)
{
  struct dirent *ned;

  if((ned=readdir(dir))){
    printf("ned->d_name = %s\n",ned->d_name);
    return ned->d_name;
  }
  else return NULL;
}

static void free_ppchar(char **n, int size)
{
  while(size!=0){
    free(n[size]);
    size--;
  }
  free(n);

  return;
}

static int qsort_cmp(const void *a, const void *b) 
{ 
  return strcmp(*(char **)a, *(char **)b);
}
