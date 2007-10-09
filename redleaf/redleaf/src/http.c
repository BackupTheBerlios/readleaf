/*http 1.1 implementation*/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <http.h>

#define _DEBUG_  1

/*TODO: exchange malloc/free/strdup/strndup to safe internal functions*/
struct http_request *parse_http_request(char *msg)
{
  struct http_request *p=NULL;
  char *tbuf=msg;
  char *ll=NULL,*pp;
  int n=0,t=0,al=0,k=0;

  if(!msg) {
#ifdef _DEBUG_
    fprintf(stderr,"The request is NULL\n");
#endif
    return NULL;
  }
  while(*tbuf!='\0') {
    if(*tbuf=='\n')
      n++;
    *tbuf++;
  }
  tbuf=msg;
  p=malloc(sizeof(struct http_request));
  if(!n) {
    p->op_code=BAD_REQUEST;
    return p;
  }
  ll=malloc(sizeof(char)*4096);
  memset(ll,'\0',4096); pp=ll;
  /*parse the header of request*/
  while(*tbuf++!='\n') al++;
  al+=1;  tbuf=msg;
  while(al!=0) {
    *pp=*tbuf;
    if(*pp==' ' || *pp=='\n') {
      switch(t) {
      case 0:
	p->method=strdup(ll);
	break;
      case 1:
	p->location=strdup(ll);
	break;
      case 2:
	if(strncmp((const char*)ll,"HTTP",4)) {
#ifdef _DEBUG_
	  fprintf(stderr,"Invalid stroke '%s'\n",ll);
#endif
	  free(ll);
	  p->op_code=BAD_REQUEST;
	  return p;
	}
	pp=ll;
	while(*pp!='\0' && *pp!='/')  *pp++;
	*pp++;
	p->pver=strndup(pp,3);
	break;
      default:
	break;
      }
      memset(ll,'\0',4096); pp=ll; t++;
    } else
      *pp++;
    *tbuf++; al--;
  }
  if(t<3) {
#ifdef _DEBUG_
    fprintf(stderr,"Invalid head of request.\n");
#endif
    p->op_code=BAD_REQUEST;
    free(ll);
    return p;
  } 
  pp=ll; memset(ll,'\0',4096); al=0;
  int st=0,j=0,q;
  /*parse the next strokes*/
  n-=2;  p->vlist=n;
  p->vars=malloc(sizeof(char)*n);
  p->values=malloc(sizeof(char)*n);
  while(*tbuf!='\0') {
    *pp=*tbuf;
    if(*pp=='\n') {
      pp=ll; st=0; j=0;
      while(al!=0) {
	if(*pp==':' && !st) {
	  if(n>=k+1) {
	    p->vars[k]=strndup(ll,sizeof(char)*j); 
	  } else {
#ifdef _DEBUG_
	    fprintf(stderr,"Request is invalid.\n");
#endif
	    free(ll);
	    p->op_code=BAD_REQUEST;
	    return p;
	  }
	  st++; q=al; al=0;
	} else if (!st) {
	  *pp++; al--; j++;
	}
      }
      if(!st && n!=k) {
#ifdef _DEBUG_
	fprintf(stderr,"Request is invalid.\n");
#endif
	free(ll);
	p->op_code=BAD_REQUEST;
	return p;
      }
      while(*++pp==' ') al++; q-=1;
      p->values[k]=strndup(pp,sizeof(char)*q-al); k++;
      pp=ll; memset(ll,'\0',4096); al=0;
    } else {
      *pp++; al++;
    }
    *tbuf++;
  }
  p->op_code=OK;
  free(ll);
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

int process_request(struct http_request *r,int fd)
{

  return 0;
}
