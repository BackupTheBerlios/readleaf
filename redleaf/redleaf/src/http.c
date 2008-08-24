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

#include "../config.h"

#ifdef MODULAS
#include <modula.h>
#endif

#define _DEBUG_  1

/*page macros*/
#define NOTFOUND_PAGE  "<html><head><title>NOT FOUND on this server.</title></head> \
<body><h1>Requested document not found.</h1><hr>%s not found.<hr>RedLeaf v0.2a</body></html>"
#define FORBIDDEN_PAGE  "<html><head><title>FORBIDDEN.</title></head> \
<body><h1>Forbidden.</h1><hr>%s access is stricted.<hr>RedLeaf v0.2a</body></html>"
#define INTERNAL_SERVER_ERROR_PAGE  "<html><head><title>Internal server error \
</title></head><body><h1>Internal Server Error</h1><hr>RedLeaf v0.2a</body>"
#define MOVED_PERMANENTLY_HEAD  "HTTP/1.1 301 Moved Permanently\n\
Date: %s\n\
Server: Redleaf\n\
Location: http://%s%s\n\
Content-Type: text/html; charset=utf-8\n\n"
#define MOVED_PERMANENTLY_PAGE  "<html><head><title>Moved permanently</title></head><body>\
<h1>Moved Permanently</h1>http://%s%s<hr>RedLeaf v0.2a</body></html>"


/*bad request page, doesn't cached it's a static*/
struct page_t *bad_request_page = NULL;

/*functions prototypes*/

/*local functions prototypes*/
static void init_http_request(struct http_request *p);
static char *mime_type(char *path);
static struct page_t *generate_bad_request_page(void);
static void gen_error_page(struct page_t *page,int err);
static void gen_notify_page(struct page_t *page, int err,
			    char *ar0,struct http_request *r);
static void update_page(struct page_t *page,struct stat ystat);
static void decode_uri(char *uri);
static inline char *skip_blanks(char *p);
static inline char *skip_nblanks(char *p);
static char *read_get_query(char *p1,char *p2);
static void __http_request_clear(struct http_request *p);

static char *read_get_query(char *p1,char *p2)
{
  int len = p2-p1;
  char *ptr=NULL;

  if(len<1)
    return NULL;
  
  ptr = rl_malloc(sizeof(char)*len+1);
  
  memset(ptr,'\0',len+1);
  strncat(ptr,p1,len);
  
  return ptr;
}

#define vstrfil(p,len,tb) p=rl_malloc(len+1); memset(p,'\0',len+1); p=strncat(p,tb,len);

struct http_request *parse_http_request(struct http_request *p,const char *msg)
{
  /*  struct http_request *p=NULL;*/
  char *tmsg=(char *)msg,*ttmsg=NULL,*tttmsg=NULL;
  char *prst,*pprst;
  int len=0,chs=0;
  char tsb;

  /*  p=rl_malloc(sizeof(struct http_request));*/
  init_http_request(p);

  if(!msg || strlen(msg)<6) {
    fprintf(stderr,"Request is NULL or empty\n");
    goto bad_request;
  }
  if(!strncmp(msg,"GET",3))
    p->method=GET;
  else if(!strncmp(msg,"POST",4))
    p->method=POST;
  else if(!strncmp(msg,"HEAD",4))
    p->method=HEAD;
  else {
  bad_request:
    p->op_code=BAD_REQUEST;
    return p;
  }

  tmsg=skip_nblanks((char *)msg); /* get URI */
  tmsg=skip_blanks(tmsg);
  ttmsg=skip_nblanks(tmsg);
  tsb=*ttmsg;*ttmsg='\0';
  tttmsg=strchr(tmsg,'?');
  if(tttmsg!=NULL) { /*get GET method query*/
    p->get_query=read_get_query(tttmsg+1,ttmsg);
    p->uri=read_get_query(tmsg,tttmsg);
  } else
    p->uri=read_get_query(tmsg,ttmsg);
  *ttmsg=tsb;

  tmsg=skip_nblanks(tmsg);
  tmsg=skip_blanks(tmsg);
  
  if(strncmp(tmsg,"HTTP/1.1",8) && strncmp(tmsg,"HTTP/1.0",8)) /*check protocol and version*/
    goto bad_request;
  tmsg=skip_nblanks(tmsg); /*get host value*/
  tmsg=skip_blanks(tmsg);

  while(tmsg!=NULL) {
    if(!strncmp(tmsg,"Host:",5)) chs=0;
    else if(!strncmp(tmsg,"User-Agent:",11)) chs=1;
    else if(!strncmp(tmsg,"Accept:",7)) chs=2;
    else if(!strncmp(tmsg,"Accept-Language:",16)) chs=3;
    else if(!strncmp(tmsg,"Accept-Encoding:",16)) chs=4;
    else if(!strncmp(tmsg,"Accept-Charset:",15)) chs=5;
    else if(!strncmp(tmsg,"Keep-Alive:",11)) chs=6;
    else if(!strncmp(tmsg,"Connection:",11)) chs=7;
    else if(!strncmp(tmsg,"Referer:",8)) chs=8;
    else if(!strncmp(tmsg,"Range:",6)) chs=9;
    else if(!strncmp(tmsg,"Cookie:",7)) chs=10;
    else if(!strncmp(tmsg,"X-",2)) chs=100;
    else chs=100;
    if(chs==100) break;
    tmsg=skip_nblanks(tmsg);tmsg=skip_blanks(tmsg);
    for(ttmsg=strchr(tmsg,'\r');ttmsg[1]!='\n';ttmsg=strchr(tmsg,'\r')) {
      if(!ttmsg)
	goto bad_request;
    }
    len=(int)(ttmsg-tmsg); 
    switch(chs) {
    case 0:      vstrfil(p->host,len,tmsg);      break;
    case 1:      vstrfil(p->user_agent,len,tmsg);      break;
    case 2:      vstrfil(p->accept,len,tmsg);      break;
    case 3:      vstrfil(p->accept_language,len,tmsg);      break;
    case 4:      vstrfil(p->accept_encoding,len,tmsg);      break;
    case 5:      vstrfil(p->accept_charset,len,tmsg);      break;
    case 8:      vstrfil(p->referer,len,tmsg);      break;
    case 10:      vstrfil(p->cookie,len,tmsg);      break;
    case 9:
      prst=strchr(tmsg,'=');
      prst++;
      pprst=strchr(prst,'-');
      len=(int)(pprst-prst);
      vstrfil(pprst,len,prst);
      p->range=atol(pprst);
      rl_free(pprst);
      break;
    }
    tmsg=skip_blanks(ttmsg);
  }

  return p;
}

/*sessions*/
http_session_t *http_session_open(http_session_t *http,in_addr_t addr)
{

  http->ht_req.cl_addr=addr; /*first we know client's address*/
  http->stream=scstr_new(4096);
  http->state=HS_INIT;

  return http;
}

http_control_t http_session_process(http_session_t *http,void *buf,size_t len)
{
  int __len;
  char *screq,*uu;

  switch(http->state) {
  case HS_INIT:
    if(!len) { /*oops, request is not correct*/
      http->ht_req.op_code=BAD_REQUEST;
      return CX_PAGE_READY;
    }
    http->stream=scstr_addn(http->stream,buf,len);
    __len=scstr_len(http->stream);
    screq=scstr_get(http->stream);
    uu=strstr(screq,"\r\n\r\n");
    if(uu && __len<HTTP_HEAD_LEN_MAX)      {
      http->state=HS_HEAD;
      goto __hs_head;
    }
    else if(__len>HTTP_HEAD_LEN_MAX) {
      http->state=HS_DONE;
      http->ht_req.op_code=BAD_REQUEST;
      return CX_PAGE_READY;
    }
    break;
  case HS_HEAD:
  __hs_head:
    __len=scstr_len(http->stream);
    screq=scstr_get(http->stream);
    parse_http_request(&http->ht_req,(const char*)screq);
    if(http->ht_req.method==POST) {
      http->state=HS_BODY;
      return CX_PAGE_READ_BODY;
    } else {
      http->state=HS_DONE;
      return CX_PAGE_READY;
    }
    break;
  case HS_BODY:
    /*gen page*/
    return CX_NO;
    break;
  case HS_DONE:
    return CX_PAGE_READY;
    break;
  }

  return CX_NO;
}

struct page_t *http_session_gen_page(http_session_t *sess)
{
  return page_t_generate(&sess->ht_req);
}

http_session_t *http_session_close(http_session_t *ss)
{

  scstr_remove(ss->stream);
  ss->state=HS_DONE;
  __http_request_clear(&ss->ht_req);

  return ss;
}

struct page_t *page_t_generate(struct http_request *ht_req)
{
  char *req;
  struct page_t *page=NULL;
#if 0
  struct http_request *ht_req=parse_http_request(request);
#endif
  struct stat ystat;
  char *filename=NULL,*tfn=NULL;
  char *root_dir=get_general_value("root_dir"),*chkfile=NULL;
  int yys=0;
#ifdef MODULAS
  modula_t *mmod=NULL;
  modula_session_t *emod;
#endif

  if(ht_req->op_code==BAD_REQUEST) {
    if(!bad_request_page)
      bad_request_page=generate_bad_request_page();
    free_http_request(ht_req);
    return bad_request_page;
  }
  req=ht_req->uri;
  decode_uri(req);
#ifdef MODULAS
  if((mmod=modula_lookup(mime_type(req)))!=NULL) { /*modula need to be processed*/
    emod=rl_malloc(sizeof(modula_session_t));
    if(mmod->modula_check_capatibilies(MOD_OPEN)!=ABSENT) {
      char *head=rl_malloc(128);
      char *date=get_rfc1123date(time(NULL));
      char *real_path=NULL;
      int ost=0;

      /*find real_path*/
      yys=sizeof(char)*(strlen(req)+strlen("/")+strlen(root_dir)+2);
      real_path=rl_malloc(yys);
      if(strcmp(req,"/")) {
	req+=sizeof(char);
	snprintf(real_path,yys,"%s%s",root_dir,req); req-=sizeof(char);
      } else tfn=strdup(root_dir);
      ht_req->real_path=real_path;
      ost=mmod->modula_session_open(mmod,emod,ht_req,NULL);
      if(ost==-1) {
	rl_free(date);
	goto _err_moda;
      }
      /*let's generate default header*/
      snprintf(head,127,"HTTP/1.1 200 OK\nDate: %s\nServer: Redleaf\n",date);
      rl_free(date);
      page=create_page_t(req,head,NULL,real_path,OK);
      page->op=4; /*ahha, it's a modula*/
      page->emod=emod;
      page->bodysize=0;
      page->head_len=strlen(head);
      goto return_and_exit;
    } else {
    _err_moda:
      rl_free(emod);
      gen_error_page(page,INTERNAL_SERVER_ERROR);
      normalize_page(page);
      goto return_and_exit;
    }

  }
#endif

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
    size_t length=strlen(req)+sizeof(char)*8;
    char *uri=rl_malloc(length);
    char *date=get_rfc1123date(time(NULL));
    char *mdate=NULL;
    char *head=rl_malloc(sizeof(char)*512);
    int fd;

    memset(uri,'\0',length);
    uri=strncpy(uri,req,length-8);

    /*look out for an index*/
    if(!root_dir) { /*get real requested filename*/
      tfn=getcwd(tfn,0);
      tfn=rl_realloc(tfn,strlen(tfn)+strlen(req)+sizeof(char)*2);
      tfn=strcat(tfn,req);
    } else {
      yys=sizeof(char)*(strlen(req)+strlen("/")+strlen(root_dir)+2);
      tfn=rl_malloc(yys);
      if(strcmp(req,"/")){
	req+=sizeof(char);
	snprintf(tfn,yys,"%s%s",root_dir,req); req-=sizeof(char);
      } else tfn=strdup(root_dir);
    }
    if(stat(tfn,&ystat)==-1) {
      page=create_page_t(uri,NULL,NULL,tfn,OK);
      rl_free(date);
      insert_cache(page);
      goto error_lock;
    }
    if(S_ISDIR(ystat.st_mode)) { /*is dir,check for index*/
      if(tfn[strlen(tfn)-1]!='/') { /*oops,was moved*/
	page=create_page_t(uri,NULL,NULL,tfn,OK);
	uri=strcat(uri,"/");
	gen_notify_page(page,MOVED_PERMANENTLY,uri,ht_req);
	normalize_page(page);
	rl_free(date);
	goto return_and_exit;
      }
      yys+=strlen("index.html")+2*sizeof(char);
      chkfile=rl_malloc(yys);

      snprintf(chkfile,yys,"%s%s",tfn,"index.html");
      if(stat(chkfile,&ystat)==-1) { /*no index*/
	filename=tfn;
	rl_free(chkfile);
      } else {
	filename=chkfile;
	rl_free(tfn);
      }
    } else filename=tfn;
    
    norm_slash_uri(filename);
    
    page=create_page_t(uri,NULL,NULL,filename,OK);

    if(stat(page->filename,&ystat)==-1) { /*oops*/
      rl_free(date);
      insert_cache(page);
      goto error_lock;
    } else {
      page->last_stat=ystat.st_mtime;
      if(S_ISDIR(ystat.st_mode)) { /*TODO: check if the dir is visible*/
	page->op=3;
	page->last_stat=ystat.st_mtime;
	snprintf(head,256,"HTTP/1.1 200 OK\nDate: %s\nServer: Redleaf\nConnection-type:"\
		 " closed\nContent-type: text/html\n\n",
		 date);
	page->head=head;
	page->head_len=strlen(head);
	page->body=read_dir_contents(filename,uri);
	page->bodysize=strlen(page->body);
      } else if(S_ISREG(ystat.st_mode)) { /*yep, file*/
	if(ht_req->range>0) {
	  /*TODO: make a check for length!*/
	  length=ystat.st_size-ht_req->range;
	  mdate=get_rfc1123date(ystat.st_mtime);
	  snprintf(head,511,"HTTP/1.1 206 Partial Content\nDate: %s\nServer: Redleaf\nLast-Modified: %s\n"\
		   "Connection-Range: bytes %ld-%ld/%ld\nContent-Length: %ld\n"\
		   "Content-Type: %s\n\n",
		   date,mdate,ht_req->range,ystat.st_size-1,ystat.st_size,length,
		   mime_type(filename));
	  page->range=ht_req->range; /*mark as partial*/
	  rl_free(mdate);
	} else {
	  length=ystat.st_size;
	  snprintf(head,511,
		   "HTTP/1.1 200 OK\nDate: %s\nServer: Redleaf\nConnection-Type:"\
		   " closed\nContent-Length: %ld\nContent-Type: %s\n\n",
		   date,(long int)length,mime_type(filename));
	}
	page->head=head;
	page->head_len=strlen(head);
	if(length+ht_req->range<=4096) { /*read this*/
	  page->op=1;
	  page->bodysize=length;
	  fd=open(filename,O_RDONLY); /*TODO: make check*/
	  page->body=rl_malloc(length);
	  read(fd,page->body,length);
	  close(fd);
	} else { /*need to be ridden from block device*/
	  page->body=NULL;
	  page->bodysize=0;
	  page->op=2;
	}
      }
    }
    rl_free(date);
    normalize_page(page);
    insert_cache(page);
  }

 return_and_exit:
#if 0
  free_http_request(ht_req);
#endif
  return page;
}

void free_http_request(struct http_request *p)
{
  if(!p)
    return;

  if(p->uri)    rl_free(p->uri);
  if(p->host)    rl_free(p->host);
  if(p->user_agent) rl_free(p->user_agent);
  if(p->accept) rl_free(p->accept);
  if(p->accept_language) rl_free(p->accept_language);
  if(p->accept_encoding) rl_free(p->accept_encoding);
  if(p->accept_charset) rl_free(p->accept_charset);
  if(p->referer) rl_free(p->referer);
  if(p->cookie) rl_free(p->cookie);
  if(p->get_query)     rl_free(p->get_query);
  p->get_query=NULL;
#ifdef MODULAS
  if(p->real_path) rl_free(p->real_path);
#endif
  rl_free(p);

  return;
}

static void __http_request_clear(struct http_request *p)
{
  if(!p)
    return;

  if(p->uri)    rl_free(p->uri);
  if(p->host)    rl_free(p->host);
  if(p->user_agent) rl_free(p->user_agent);
  if(p->accept) rl_free(p->accept);
  if(p->accept_language) rl_free(p->accept_language);
  if(p->accept_encoding) rl_free(p->accept_encoding);
  if(p->accept_charset) rl_free(p->accept_charset);
  if(p->referer) rl_free(p->referer);
  if(p->cookie) rl_free(p->cookie);
  if(p->get_query)     rl_free(p->get_query);
  p->get_query=NULL;
  p->uri=p->host=p->user_agent=p->accept=p->accept_language=NULL;
  p->accept_encoding=p->accept_charset=p->referer=p->cookie=NULL;
#ifdef MODULAS
  if(p->real_path) rl_free(p->real_path);
#endif
  p->real_path=NULL;

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
  char *body="<html><head><title>400 Bad Request:</title></head><body><h1>Bad Request</h1><br>Your client made unresolved request.<hr>Redleaf v0.1b</body></html>";
  int head_len=strlen(head);
  int body_len=strlen(body);
  bad_request_page=rl_malloc(sizeof(struct page_t));
  struct page_t *page=bad_request_page;

  page->uri=NULL;
  data=rl_malloc(head_len+body_len+1);
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
  if(page->body)    rl_free(page->body);
  if(page->head)    rl_free(page->head);

  page->head=rl_malloc(sizeof(char)*256);

  switch(err) {
  case NOT_FOUND:
    snprintf(page->head,255,"HTTP/1.1 404 Not Found\nDate: %s\nServer: Redleaf\nConnection-type: closed\nContent-type: text/html\n\n",date);
    len=strlen(NOTFOUND_PAGE)+strlen(page->uri)+sizeof(char);
    page->body=rl_malloc(len);
    snprintf(page->body,len-1,NOTFOUND_PAGE,page->uri);
    page->head_len=strlen(page->head);
    page->bodysize=strlen(page->body);
    break;
  case FORBIDDEN:
    snprintf(page->head,255,"HTTP/1.1 403 Forbidden\nDate: %s\nServer: Redleaf\nConnection-type: closed\nContent-type: text/html\n\n",date);
    len=strlen(FORBIDDEN_PAGE)+strlen(page->uri)+sizeof(char);
    page->body=rl_malloc(len);
    snprintf(page->body,len-1,FORBIDDEN_PAGE,page->uri);
    page->head_len=strlen(page->head);
    page->bodysize=strlen(page->body);
    break;
  case MOVED_PERMANENTLY:

    break;
  case INTERNAL_SERVER_ERROR:
  default:
    snprintf(page->head,255,"HTTP/1.1 500 Internal Server Error\nDate: %s\nServer: Redleaf\nConnection-type: closed\nContent-type: text/html\n\n",date);
    len=strlen(INTERNAL_SERVER_ERROR_PAGE)+sizeof(char);
    page->body=rl_malloc(len);
    snprintf(page->body,len-1,INTERNAL_SERVER_ERROR_PAGE);
    page->head_len=strlen(page->head);
    page->bodysize=strlen(page->body);
    break;
  }

  rl_free(date);

  return;
}

static void gen_notify_page(struct page_t *page, int err,char *ar0,struct http_request *r)
{
  char *date=get_rfc1123date(time(NULL));
  int len=0;

  if(page->head)
    denormalize_page(page);
  page->op=err;
  if(page->body)    rl_free(page->body);
  if(page->head)    rl_free(page->head);

  switch(err) {
  case MOVED_PERMANENTLY:
    len=strlen(ar0)+strlen(r->host)+strlen(date);
    len+=strlen(MOVED_PERMANENTLY_HEAD)+(2*sizeof(char));
    page->head=rl_malloc(len);
    snprintf(page->head,len-1,MOVED_PERMANENTLY_HEAD,date,r->host,ar0);
    len-=strlen(MOVED_PERMANENTLY_HEAD)+strlen(date);
    len+=strlen(MOVED_PERMANENTLY_PAGE);
    page->body=rl_malloc(len);
    snprintf(page->body,len-1,MOVED_PERMANENTLY_PAGE,r->host,ar0);
    page->head_len=strlen(page->head);
    page->bodysize=strlen(page->body);
    break;
  }

  rl_free(date);

  return;
}

static void update_page(struct page_t *page,struct stat ystat)
{
  char *date=get_rfc1123date(time(NULL));
  int fd;

  denormalize_page(page);

  if(page->body)    rl_free(page->body);
  if(page->head)    rl_free(page->head);
  page->head=rl_malloc(256*sizeof(char));
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
    page->body=rl_malloc(ystat.st_size);
    fd=open(page->filename,O_RDONLY);
    read(fd,page->body,ystat.st_size);
    close(fd); 
    page->op=1;
  }

  rl_free(date);

  return;
}

static void init_http_request(struct http_request *p)
{
  if(!p)
    return;

  p->uri=p->host=p->user_agent=p->accept=p->accept_language=p->accept_encoding=NULL;
  p->cookie=p->accept_charset=p->referer=p->get_query=NULL;
#ifdef MODULAS
  p->real_path=NULL;
#endif
  p->op_code = OK;
  p->range=0;

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

static inline char *skip_blanks(char *p)
{
  if(!p)
    return NULL;
  while(isspace(*p))
    p++;

  return p;
}

static inline char *skip_nblanks(char *p)
{
  if(!p)
    return NULL;
  while(*p && !isspace(*p))
    p++;

  return p;
}

