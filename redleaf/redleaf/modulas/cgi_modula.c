/*
 * RedLeafd cgi modula
 *
 * Copyright (C) 2006, 2007, 2008 ReadLeaf devteam org.
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

/*redleafd includes*/
#include <conf.h>
#include <http.h>
#include <modula.h>
#include <misc.h>
#include <libdata/usrtc.h>

const modula_info_t cgi_modula_info = {
  .name="cgi_modula",
  .author="tirra",
  .version="0.1beta",
};

static const int op_cap_codes[] = { 
  IMPLEMENTED,
  IMPLEMENTED,
  IMPLEMENTED,
  IMPLEMENTED,
  ABSENT,
};

/*symbols that called by default*/
void _init(void)
{
  fprintf(stdout,"<<MODULA>> %s ver. %s by '%s' initied.\n",cgi_modula_info.name,
	  cgi_modula_info.version,cgi_modula_info.author);

  return;
}

void _fini(void)
{
  fprintf(stdout,"<<MODULA>> %s shooted out.\n",cgi_modula_info.name);

  return;
}

/*standart functions*/

/*======general functions======**/
int cgi_modula_init(modula_t *modula, void *data)
{
  if(!modula) {
    fprintf(stderr,">>MODULA<< %s_init: given pointer is nil.\n",cgi_modula_info.name);
    return -1;
  }

  modula->info=cgi_modula_info;

  return 0;
}

int cgi_modula_shootout(modula_t *modula)
{

  return 0;
}

/*session functions*/
int cgi_modula_session_open(modula_t *modula,modula_session_t *session,
			    struct http_request *ht_req,void *data)
{
  struct stat ystat;
  pid_t pid;
  int sout[2],sin[2];
  int cstate;

  if(!modula || !session)
    return -1;

  if(!ht_req) {
    fprintf(stderr,"<<MODULA>> %s: http request is nil.\n",cgi_modula_info.name);
    return -1;
  }

  /*check for real_path*/
  if(stat(ht_req->real_path,&ystat)<0) {
    fprintf(stderr,"<<MODULA>> %s: cannot stat `%s'.\n",cgi_modula_info.name,ht_req->real_path);
    return -1;
  }
  if(!(ystat.st_mode | S_IXUSR)) { /*can it be ?*/
    fprintf(stderr,"<<MODULA>> %s: cannot execute `%s'.\n",cgi_modula_info.name,ht_req->real_path);
    return -1;
  } 
  /*okay let's fill structures*/
  session->modula_session_close=modula->modula_session_close;
  session->modula_session_read=modula->modula_session_read;
  session->modula_session_write=modula->modula_session_write;
  /*now let's initiate session*/
  
  if(pipe(sout)==-1) {
    fprintf(stderr,"<<MODULA>> %s: error opening pipe for stdout.\n",cgi_modula_info.name);
    return -1;
  }
  if(pipe(sin)==-1) {
    fprintf(stderr,"<<MODULA>> %s: error opening pipe for stdin.\n",cgi_modula_info.name);
    return -1;
  }

  pid=fork();
  if(pid==-1) {
    fprintf(stderr,"<<MODULA>> %s: error creating fork().\n",cgi_modula_info.name);
    return -1;
  }
  /*what will done child*/
  if(!pid) {
    int i,ii,o=0,wdlen,pairs=0;
    char *d=strrchr(ht_req->real_path,'/'),*wd,*va,*val;

    close(sout[0]); /*stdout*/
    close(1);
    i=dup(sout[1]);
    close(sout[1]);

    close(sin[0]); /*stdin*/
    close(0);
    ii=dup(sin[1]);
    close(sin[1]);

    /*server name*/
    setenv("SERVER_NAME","redleafd",1);

    /*set working directory*/
    wdlen=d-ht_req->real_path;
    wd=rl_malloc(wdlen+1);
    memset(wd,'\0',wdlen+1);
    strncat(wd,ht_req->real_path,wdlen);
    chdir(wd);     rl_free(wd);

    /*set http defaults variables*/
    if(ht_req->uri)      setenv("URI",ht_req->uri,1);
    if(ht_req->host)      setenv("HOST",ht_req->host,1);
    if(ht_req->user_agent)      setenv("User-Agent",ht_req->host,1);
    if(ht_req->accept)      setenv("Accept",ht_req->accept,1);
    if(ht_req->accept_language)      setenv("Accept-Language",ht_req->accept_language,1);
    if(ht_req->accept_encoding)      setenv("Accept-Encoding",ht_req->accept_encoding,1);
    if(ht_req->accept_charset)      setenv("Accept-Charset",ht_req->accept_charset,1);
    if(ht_req->referer)      setenv("Referer",ht_req->referer,1);
    if(ht_req->cookie)      setenv("Cookie",ht_req->cookie,1);
    if(ht_req->get_query)      setenv("GET_STRING",ht_req->get_query,1);

    /*ok,now we will parse other env*/
    switch(ht_req->method) {
    case GET: /*GET method support*/
      setenv("METHOD","GET",1);
      if(ht_req->get_query) {
	setenv("GET",(const char*)ht_req->get_query,1);
	wd=ht_req->get_query;
	while(*wd!='\0') {
	  if(*wd=='=')	    pairs++;
	  wd+=sizeof(char);
	}
	if(pairs) {
	  wd=ht_req->get_query;
	  while(pairs!=0) {
	    if(*wd=='\0') break;
	    d=strchr(wd,'='); /*variable name*/
	    if(!d)	      d=strchr(wd,'\0');
	    wdlen=(int)(d-wd);
	    va=rl_malloc(wdlen+1);
	    memset(va,'\0',wdlen+1);
	    strncat(va,wd,wdlen);
	    wd=d+sizeof(char); /*variable value*/
	    if(*wd=='\0')
	      break;
	    d=strchr(wd,'&');
	    if(!d)	      d=strchr(wd,'\0');
	    wdlen=(int)(d-wd);
	    val=rl_malloc(wdlen+1);
	    memset(val,'\0',wdlen+1);
	    strncat(val,wd,wdlen);
	    /*setenv*/
	    setenv((const char*)va,(const char *)val,1);
	    rl_free(va);rl_free(val);
	    /*seek*/
	    wd=d+sizeof(char);
	    pairs--;
	  }
	}
      }
      break;
    case HEAD:
      break;
    case POST:
      setenv("METHOD","POST",1);
      break;
    }

    if((o=execl(ht_req->real_path,ht_req->real_path,NULL))==-1) {
      perror("execl:");
      exit(3);
    }
    return 0;
  } else {
    waitpid(pid,&cstate,0);
    close(sout[1]);
    close(sin[1]);

    /*set descriptors*/
    session->pipe_rd=sout[0]; /*stdout*/
    session->pipe_wr=sin[0];  /*stdin*/
  }

  return 0;
}

int cgi_modula_session_close(modula_session_t *session)
{
  if(!session)
    return -1;

  fprintf(stderr,"<<MODULA>> DD: closing session(%d).\n",session->pipe_rd);

  if(close(session->pipe_rd)==-1) {
    fprintf(stderr,"<<MODULA>> DD: closing session error (%d is invalid)[stdout].\n",session->pipe_rd);
    perror("close:");
    return -1;
  }
  if(close(session->pipe_wr)==-1) {
    fprintf(stderr,"<<MODULA>> DD: closing session error (%d is invalid)[stdin].\n",session->pipe_wr);
    perror("close:");
    return -1;
  }

  return 0;
}

size_t cgi_modula_session_read(modula_session_t *session,void *buf,size_t size)
{
  if(!session)
    return -1;

  return read(session->pipe_rd,buf,size);
}

size_t cgi_modula_session_write(modula_session_t *session,void *buf,size_t size)
{
  if(!session)
    return -1;

  return write(session->pipe_wr,buf,size);
}

/*misc functions*/
inline int cgi_modula_check_capatibilies(int op_code)
{
  return op_cap_codes[op_code];
}


