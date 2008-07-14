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
#include <ctype.h>
#include <conf.h>

#define _DEBUG_  1

/*page macros*/
#define NOTFOUND_PAGE  "<html><head><title>NOT FOUND on this server.</title></head> \
<body><h1>Requested document not found.</h1><hr>%s not found.<hr>RedLeaf v0.1a</body></html>"
#define FORBIDDEN_PAGE  "<html><head><title>FORBIDDEN.</title></head> \
<body><h1>Forbidden.</h1><hr>%s access is stricted.<hr>RedLeaf v0.1a</body></html>"
#define INTERNAL_SERVER_ERROR_PAGE  "<html><head><title>Internal server error \
</title></head><body><h1>Internal Server Error</h1><hr>RedLeaf v0.1a</body>"

/*bad request page, doesn't cached it's a static*/
struct page_t *bad_request_page = NULL;

/*functions prototypes*/
void free_http_reply(struct http_reply *reply);

/*local functions prototypes*/
static void init_http_request(struct http_request *p);
static char *mime_type(char *path);
static struct page_t *generate_bad_request_page(void);
static void gen_error_page(struct page_t *page,int err);
static void update_page(struct page_t *page,struct stat ystat);
static void decode_uri(char *uri);

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
  if(strncmp(ttmsg+sizeof(char),"HTTP/1.1",8) && strncmp(ttmsg+sizeof(char),"HTTP/1.0",8)) 
    goto bad_request;

  return p;
}

struct page_t *page_t_generate(char *request)
{
  char *req;
  struct page_t *page=NULL;
  struct http_request *ht_req=parse_http_request(request);
  struct stat ystat;
  char *filename=NULL,*tfn=NULL;
  char *root_dir=get_general_value("root_dir"),*chkfile=NULL;
  int yys=0;

  if(!ht_req) {
    fprintf(stderr,"Request not parsed!\n");
    if(!bad_request_page)
      bad_request_page=generate_bad_request_page();
    return bad_request_page;
  }

  if(ht_req->op_code==BAD_REQUEST) {
    if(!bad_request_page)
      bad_request_page=generate_bad_request_page();
    free_http_request(ht_req);
    return bad_request_page;
  }
  req=ht_req->uri;
  decode_uri(req);

  page=lookup_cache(req);
  if(page) { /*located in cache*/
    if(page->ref!=0) /*if it's currently used via connection - return it*/
      goto return_and_exit;
    if(page->last_access + CACHE_TIMEOUT > time(NULL)) /*return if not timeouted*/
      goto return_and_exit;
    /*in other cases need to check if exist, the same*/
    if(stat(page->filename,&ystat)==-1){ /*oops*/
    error_lock:
      switch(errno) {
      case EACCES:
	gen_error_page(page,FORBIDDEN); 
	break;
      case ENOENT:
      case ELOOP:
      case EFAULT:
	gen_error_page(page,NOT_FOUND);
	break;
      default:
	gen_error_page(page,INTERNAL_SERVER_ERROR);
	break;
      }
      normalize_page(page);
      goto return_and_exit;
    } else {
      if(page->last_stat==ystat.st_mtime) /*the same*/
	goto return_and_exit;
      update_page(page,ystat);
      normalize_page(page);
    }
  } else { /*so, need to be generated*/
    char *uri=strdup(req);
    char *date=get_rfc1123date(time(NULL));
    char *head=malloc(sizeof(char)*512);
    size_t length;
    int fd;

    /*look out for an index*/
    if(!root_dir) { /*get real requested filename*/
      tfn=getcwd(tfn,0);
      tfn=rl_realloc(tfn,strlen(tfn)+strlen(req)+sizeof(char)*2);
      tfn=strcat(tfn,req);
    } else {
      yys=sizeof(char)*(strlen(req)+strlen("/")+strlen(root_dir)+2);
      tfn=rl_malloc(yys);
      if(!tfn) {
	fprintf(stderr,"Error allocating memory.(page_t_generate)\n");
	exit(3);
      }
      if(strcmp(req,"/")){
	req+=sizeof(char);
	snprintf(tfn,yys,"%s%s",root_dir,req); req-=sizeof(char);
      } else tfn=strdup(root_dir);
    }
    if(stat(tfn,&ystat)==-1)    goto error_lock;
    if(S_ISDIR(ystat.st_mode)) { /*is dir,check for index*/
      yys+=strlen("index.html")+2*sizeof(char);
      chkfile=rl_malloc(yys);
      if(!chkfile) {
	fprintf(stderr,"Error allocating memory.(page_t_generate)\n");
      }
      snprintf(chkfile,yys,"%s/%s",tfn,"index.html");
      if(stat(chkfile,&ystat)==-1) {
	filename=tfn;
	rl_free(chkfile);
      } else {
	filename=chkfile;
	rl_free(tfn);
      }
    } else filename=tfn;
    
    norm_slash_uri(filename);
    
    page=create_page_t(uri,NULL,NULL,filename,OK);
    printf("filename=%s\n",filename);

    if(stat(page->filename,&ystat)==-1) { /*oops*/
      free(date);
      insert_cache(page);
      goto error_lock;
    } else {
      page->last_stat=ystat.st_mtime;
      if(S_ISDIR(ystat.st_mode)) { /*TODO: check if the dir is visible*/
	page->op=3;
	page->last_stat=ystat.st_mtime;
	snprintf(head,256,"HTTP/1.1 200 OK\nDate: %s\nServer: Redleaf\nConnection-type: closed\nContent-type: text/html\n\n",date);
	page->head=head;
	page->head_len=strlen(head);
	page->body=read_dir_contents(filename,uri);
	page->bodysize=strlen(page->body);
      }
      if(S_ISREG(ystat.st_mode)) { /*yep, file*/
	length=ystat.st_size;
	sprintf(head,"HTTP/1.1 200 OK\nDate: %s\nServer: Redleaf\nConnection-type: closed\nContent-Length: %ld\nContent-type: %s\n\n",
		date,(long int)length,mime_type(filename));
	page->head=head;
	page->head_len=strlen(head);
	if(length<=4096) { /*read this*/
	  page->op=1;
	  page->bodysize=length;
	  fd=open(filename,O_RDONLY); /*TODO: make check*/
	  page->body=malloc(length);
	  read(fd,page->body,length);
	  close(fd);
	} else { /*need to be ridden from block device*/
	  page->body=NULL;
	  page->bodysize=0;
	  page->op=2;
	}
      }
    }
    free(date);
    normalize_page(page);
    insert_cache(page);
  }

 return_and_exit:
  free_http_request(ht_req);
  return page;
}

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

static char *_st_mime_type(char *path)
{
  char *d=NULL;
  d=strrchr(path,'.');
  if(!d)
    return "text/plain";
  else if(!strcasecmp(d,".html"))
    return "text/html";
  else if(!strcasecmp(d,".htm"))
    return "text/html";
  else if(!strcasecmp(d,".css"))
    return "text/css";
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
  else if(!strcasecmp(d,".css"))
    return "text/css";
  return "text/plain";
}

static char *mime_type(char *path)
{
  char *d=NULL;
  struct variable *u=get_mimetype_variables("type");
  struct variable *o=u;

  d=strrchr(path,'.');
  if(!d)    return "text/plain";
  if(!u)    return _st_mime_type(path);

  while(o->var!=NULL) {
    if(!strcasecmp(d,o->var))
      return o->value;
    o++;
  }

  return "text/plain";
}

static struct page_t *generate_bad_request_page(void)
{
  char *data;
  char *head="HTTP/1.1 400 Bad Request\nServer: Redleaf\nConnection-type: closed\nContent-type: text/html\n\n";
  char *body="<html><head><title>400 Bad Request:</title></head><body><h1>Bad Request</h1><br>Your client made unresolved request.<hr>Redleaf v0.1a</body></html>";
  int head_len=strlen(head);
  int body_len=strlen(body);
  bad_request_page=malloc(sizeof(struct page_t));
  struct page_t *page=bad_request_page;

  if(!bad_request_page) {
    fprintf(stderr,"Error allocating memory for buffer.(http.c:generate_bad_request()#1)\n");
    return NULL;
  }

  page->uri=NULL;
  data=malloc(head_len+body_len+1);
  if(!data) {
    fprintf(stderr,"Error allocating memory for buffer.(http.c:generate_bad_request()#2)\n");
    free(bad_request_page);
    return NULL;
  }
  snprintf(data,head_len+body_len+1,"%s%s",head,body);
  page->head=page->body=data;
  page->body+=head_len;
  page->filename=NULL;
  page->head_len=head_len;
  page->bodysize=body_len;
  page->last_modify=page->last_stat=page->last_access=time(NULL);
  page->op=BAD_REQUEST;
  page->ref=0;

  return bad_request_page;
}

static void gen_error_page(struct page_t *page,int err)
{
  char *date=get_rfc1123date(time(NULL));
  int len=0;

  if(page->head)
    denormalize_page(page);

  page->op=err;
  if(page->body)    free(page->body);
  if(page->head)    free(page->head);

  page->head=malloc(sizeof(char)*256);

  switch(err) {
  case NOT_FOUND:
    page->head=malloc(sizeof(char)*256);
    snprintf(page->head,255,"HTTP/1.1 404 Not Found\nDate: %s\nServer: Redleaf\nConnection-type: closed\nContent-type: text/html\n\n",date);
    len=strlen(NOTFOUND_PAGE)+strlen(page->uri)+sizeof(char);
    page->body=malloc(len);
    snprintf(page->body,len-1,NOTFOUND_PAGE,page->uri);
    page->head_len=strlen(page->head);
    page->bodysize=strlen(page->body);
    break;
  case FORBIDDEN:
    page->head=malloc(sizeof(char)*256);
    snprintf(page->head,255,"HTTP/1.1 403 Forbidden\nDate: %s\nServer: Redleaf\nConnection-type: closed\nContent-type: text/html\n\n",date);
    len=strlen(FORBIDDEN_PAGE)+strlen(page->uri)+sizeof(char);
    page->body=malloc(len);
    snprintf(page->body,len-1,FORBIDDEN_PAGE,page->uri);
    page->head_len=strlen(page->head);
    page->bodysize=strlen(page->body);
    break;
  case INTERNAL_SERVER_ERROR:
  default:
    page->head=malloc(sizeof(char)*256);
    snprintf(page->head,255,"HTTP/1.1 500 Internal Server Error\nDate: %s\nServer: Redleaf\nConnection-type: closed\nContent-type: text/html\n\n",date);
    len=strlen(INTERNAL_SERVER_ERROR_PAGE)+sizeof(char);
    page->body=malloc(len);
    snprintf(page->body,len-1,INTERNAL_SERVER_ERROR_PAGE);
    page->head_len=strlen(page->head);
    page->bodysize=strlen(page->body);
    break;
  }

  free(date);

  return;
}

static void update_page(struct page_t *page,struct stat ystat)
{
  char *date=get_rfc1123date(time(NULL));
  int fd;

  denormalize_page(page);

  if(page->body)    free(page->body);
  if(page->head)    free(page->head);
  page->head=malloc(256*sizeof(char));
  sprintf(page->head,"HTTP/1.1 200 OK\nDate: %s\nServer: Redleaf\nConnection-type: closed\nContent-Length: %ld\nContent-type: %s\n\n",
	  date,ystat.st_size,mime_type(page->uri));
  page->head_len=strlen(page->head);
  page->last_stat=ystat.st_mtime;

  if(ystat.st_size>4096) { /*big file*/
    page->body=NULL;
    page->bodysize=0;
    page->op=2;
  } else {
    page->bodysize=ystat.st_size;
    page->body=malloc(ystat.st_size);
    fd=open(page->filename,O_RDONLY);
    read(fd,page->body,ystat.st_size);
    close(fd); 
    page->op=1;
  }

  free(date);

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

static void decode_uri(char *uri)
{
  char *w,*t;

  for(w=uri,t=uri;*w;) {
    if(*w=='%' && isxdigit(w[1]) && isxdigit(w[2])) {
      *t++ =(isdigit(w[1]) ? (w[1]-'0') : 
	     (tolower(w[1]) - 'a' + 10)) * 16 +
	(isdigit(w[2]) ? (w[2] - '0') : 
	 (tolower(w[2]) - 'a' + 10));
      w+=3;
    } else
      *t++ = *w++;
  }

  *t='\0';

  return;
}
