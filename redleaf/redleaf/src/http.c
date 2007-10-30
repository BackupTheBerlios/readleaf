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
#include <page.h>

#define _DEBUG_  1

/*page macros*/
#define NOTFOUND_PAGE  "<html><head><title>NOT FOUND on this server.</title></head> \
<body><h1>Requested document not found.</h1><hr>%s not found.<hr>RedLeaf v0.1a</body></html>"
#define FORBIDDEN_PAGE  "<html><head><title>FORBIDDEN.</title></head> \
<body><h1>Forbidden.</h1><hr>%s access is stricted.<hr>RedLeaf v0.1a</body></html>"

/*functions prototypes*/
void free_http_reply(struct http_reply *reply);

/*local functions prototypes*/
static void init_http_request(struct http_request *p);
static char *mime_type(char *path);
static char *generate_reply_head(char *uri,char *filepath,int *op);
static void quick_exchange_errorpage(struct page_t *page,int op);
static void quick_exchange_filepage(struct page_t *page,struct stat ystat);
static int send_page_t(struct page_t *page,int fd);

/*TODO: exchange malloc/free/strdup/strndup to safe internal functions*/
/*TODO: parse other variables from request*/
struct http_request *parse_http_request(char *msg)
{
  struct http_request *p=NULL;
  char *tmsg=msg,*ttmsg;

  if(!(p=malloc(sizeof(struct http_request)))) {
    fprintf(stderr,"Enough memory for allocate the structure.\n");
    return NULL;
  }
  init_http_request(p);

  if(!msg || strlen(msg)<6) {
    fprintf(stderr,"Request is NULL or empty\n");
    goto bad_request;
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
    *ttmsg='\0';
    p->uri=strdup(tmsg+sizeof(char));
    *ttmsg=' ';
  } else goto bad_request;
  if(strncmp(ttmsg+sizeof(char),"HTTP/1.1",8)) 
    goto bad_request;

  return p;
}

int write_http_reply(struct http_reply *r,int fd)
{
#if 0
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
    return -1;
  }

  return wl;
#endif
return 0;
}

static int send_dir_content(const char *path,const char *uri,int fd)
{
  char buf[512];
  int wl;

  /*TODO: remake this one*/
  sprintf(buf,"<html><head><title>%s:</title></head><body>",uri);
  wl+=write(fd,buf,strlen(buf));
  sprintf(buf,"<h1>Directory %s:</h1><hr>",uri);
  wl+=write(fd,buf,strlen(buf));
  wl+=write_html_dir_list(fd,path,uri);
  sprintf(buf,"<hr>RedLeaf httpd v0.1a</body></html>");
  wl+=write(fd,buf,strlen(buf));

  return wl;
}

static int send_file_content(const char *path,int fd)
{
  /*TODO: better pipe->pipe transactions*/

  char cc;
  int cd,wl;

  cd=open(path,O_RDONLY);
  while(read(cd,&cc,sizeof(char)))
    wl+=write(fd,&cc,sizeof(char));
  close(cd);


  return wl;
}

static int send_page_t(struct page_t *page,int fd)
{
  int wl=0;

  wl+=write(fd,page->head,strlen((const char*)page->head));

  switch(page->op) {
  case 2:
    wl+=send_file_content(page->filename,fd);
    break;
  case 3:
    wl+=send_dir_content(page->filename,page->uri,fd);
    break;
  default: /*simply output*/
    wl+=write(fd,page->body,page->bodysize);
    break;
  }

  return wl;
}

/*TODO: add normal processing*/
int process_request(struct http_request *r,int fd)
{
  struct page_t *page=NULL;
  char *req;
  char *filepath=NULL;
  char *head=NULL;
  struct stat ystat;
  int op,ffd,bl;

  if(r->op_code != BAD_REQUEST) {
    req=r->uri;
    page=lookup_cache(req);
    if(page) {
      /*quick check*/
      stat(page->filename,&ystat);

      if(errno==EACCES)
	op=FORBIDDEN;
      else if(errno==ENOENT)
	op=NOT_FOUND;
      else
	op=OK;

      if(page->op>200) {

	if(op!=page->op) 
	  quick_exchange_errorpage(page,op);
	else 
	  page->last_access=time(NULL);
      } else {
	switch(page->op) {
	case 1:
	case 2:
	  if(page->last_stat!=ystat.st_mtime && op==OK) { /*fucking deal - file changed*/
	    quick_exchange_filepage(page,ystat);
	    page->last_stat=ystat.st_mtime;
	  } else if(op!=OK) 
	    quick_exchange_errorpage(page,op);
	  break;
	case 3:
	  if(op!=OK) /*directory doesn't exist*/
	    quick_exchange_errorpage(page,op);
	  break;
	}

	page->last_access=time(NULL);
      }
      
    } else { /*generate struct page_t for non-cached request*/
      req=strdup(r->uri); /*uri*/
      /*TODO: remake this for working with CM*/
      filepath=getcwd(filepath,0); /*determine filepath*/
      filepath=realloc(filepath,strlen(filepath)+strlen(req)+sizeof(char));
      filepath=strcat(filepath,req);
      head=generate_reply_head(req,filepath,&op); /*fill head*/

      page=create_page_t(req,head,NULL,filepath,op);
      if(op<200) {
	stat(filepath,&ystat);
	page->last_stat=ystat.st_mtime;
	if(op==1){
	  ffd=open(page->filename,O_RDONLY);
	  page->body=malloc(ystat.st_size);
	  read(ffd,page->body,ystat.st_size);
	  close(ffd);
	  page->bodysize=ystat.st_size;
	} else if(op==2) {
	  page->body=NULL;
	  page->bodysize=ystat.st_size;
	} else 
	  page->body=NULL;
      } else if(op==NOT_FOUND) {
	bl=sizeof(char)*(strlen(NOTFOUND_PAGE)+strlen(page->uri)+1);
	page->body=malloc(bl);
	snprintf(page->body,bl,NOTFOUND_PAGE,page->uri);
      } else if(op==FORBIDDEN){
	bl=sizeof(char)*(strlen(FORBIDDEN_PAGE)+strlen(page->uri)+1);
	page->body=malloc(bl);
	snprintf(page->body,bl,FORBIDDEN_PAGE,page->uri);
      }

      insert_cache(page);
    }
  }

  if(send_page_t(page,fd)==-1)
    return -1;

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
  if(reply->buf)
    free(reply->buf);
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
  p->op_code = OK;

  return;
}

static char *mime_type(char *path)
{
  char *d=NULL;
  d=strrchr(path,'.');
  if(!d)
    return "text/plain";
  else if(!strcasecmp(d,".html"))
    return "text/html";
  else if(!strcasecmp(d,".htm"))
    return "text/html";
  else if(!strcasecmp(d,".txt"))
    return "text/plain";
  else if(!strcasecmp(d,".text"))
    return "text/plain";
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

static char *generate_reply_head(char *uri,char *filepath,int *op)
{
  char *head=NULL;
  struct stat ystat;
  /*TODO: size determining*/
  int fd,hl=sizeof(char)*(512+strlen((const char*)uri)+sizeof(unsigned long));
  int mode=0;
  unsigned long filesize=0;
  char *date;

  head=malloc(hl);
  if(head==NULL)
    return NULL;
  /*
   * 1 - check for errors
   * 2 - determine dir or not
   * 3 - if dir use http_read_dir
   *     not determine the file type, size and decide how it will be located
   * 4 - finish misc things
   */
  if((fd=open(filepath,O_RDONLY))==-1) { /*error, check for type of error*/
    switch(errno) {
    case EACCES:
      mode=FORBIDDEN;
      break;
    default:
      mode=NOT_FOUND;
      break;
    }
    *op=mode;
  } else { /*dir or not (to) dir ;-)*/
    mode=OK;
    fstat(fd,&ystat);
    if(S_ISREG(ystat.st_mode)) {
      filesize=ystat.st_size;
      if(filesize<4096)
	*op=1;
      else
	*op=2;
    } else if(S_ISDIR(ystat.st_mode)) {
      *op=3; /*mark that it's dir*/
    }
  }

  date=get_rfc1123date(time(NULL));

  switch(*op) {
  case FORBIDDEN:
    snprintf(head,hl,"HTTP/1.1 403 Forbidden\nDate: %s\nServer: %s\n\
Connection-type: closed\nContent-type: text/html\n\n",date,(const char *)"RedLeaf v0.1a");
    break;
  case NOT_FOUND:
    snprintf(head,hl,"HTTP/1.1 404 Not Found\nDate: %s\nServer: %s\n\
Connection-type: closed\nContent-type: text/html\n\n",date,(const char *)"RedLeaf v0.1a");
    break;
  case 1:
  case 2:
    snprintf(head,hl,"HTTP/1.1 200 OK\nDate: %s\nServer: %s\n\
Content-Length:%ld\nConnection-type: closed\nContent-type: %s\n\n",date,(const char *)"RedLeaf v0.1a",
	     filesize,mime_type(uri));
    break;
  case 3:
    snprintf(head,hl,"HTTP/1.1 200 OK\nDate: %s\nServer: %s\n\
Connection-type: closed\nContent-type: text/html\n\n",date,(const char *)"RedLeaf v0.1a");
    break;
  }

  free(date);

  return head;
}

static void quick_exchange_errorpage(struct page_t *page,int op)
{
  int hl=sizeof(char)*(512+strlen((const char*)page->uri)+sizeof(unsigned long));
  int bl;
  char *date=get_rfc1123date(page->last_access);

  if(page->body)
    free(page->body);

  switch(op) {
  case NOT_FOUND:
    snprintf(page->head,hl,"HTTP/1.1 404 Not Found\nDate: %s\nServer: %s\n \
Connection-type: closed\nContent-type: text/html\n\n",date,
	     (const char *)"RedLeaf v0.1a");
    bl=sizeof(char)*(strlen(NOTFOUND_PAGE)+strlen(page->uri)+1);
    snprintf(page->body,bl,NOTFOUND_PAGE,page->uri);
    break;
  case FORBIDDEN:
    snprintf(page->head,hl,"HTTP/1.1 403 Forbidden\nDate: %s\nServer: %s\n \
Connection-type: closed\nContent-type: text/html\n\n",date,
	     (const char *)"RedLeaf v0.1a");
    bl=sizeof(char)*(strlen(FORBIDDEN_PAGE)+strlen(page->uri)+1);
    snprintf(page->body,bl,FORBIDDEN_PAGE,page->uri);
    break;
  }

  page->op=op;

  free(date);

  return;
}

static void quick_exchange_filepage(struct page_t *page,struct stat ystat)
{
  int fd;
  int hl=sizeof(char)*(512+strlen((const char*)page->uri)+sizeof(unsigned long));
  char *date=NULL;

  if(page->body)
    free(page->body);

  page->bodysize=ystat.st_size;

  /*change content if needed*/
  if(page->bodysize>4096) {
    page->body=NULL;
    page->op=2;
  }
  else {
    fd=open(page->filename,O_RDONLY);
    page->body=malloc(page->bodysize);
    read(fd,(void *)page->body,page->bodysize);
    close(fd);
    page->op=1;
  }

  /*change the head*/
  date=get_rfc1123date(ystat.st_mtime);
  snprintf(page->head,hl,"HTTP/1.1 200 OK\nDate: %s\nServer: %s\n \
Content-Length:%ld\nConnection-type: closed\nContent-type: %s\n\n",date,(const char *)"RedLeaf v0.1a",
	   page->bodysize,mime_type(page->uri));

  free(date);

  return;
}

