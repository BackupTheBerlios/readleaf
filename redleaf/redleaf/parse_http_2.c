#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#define vstrfil(p,len,tb) p=malloc(len+1); memset(p,'\0',len+1); p=strncat(p,tb,len);
#define S_S(s) (s)?(struct sub_str){s,strlen(s)}:(struct sub_str){0,0}
#define S_SL(s,l) (s || l)?(struct sub_str){s,l}:(struct sub_str){0,0}

#include <http.h>
struct sub_str{
  char *s;  /*  str  */
  int l;  /*  len str  */
};


void print_sub_str(const struct sub_str s);
static inline struct sub_str skip_blanks(const struct sub_str s);
static inline struct sub_str skip_nblanks(const struct sub_str s);
int str_case_cmp(const struct sub_str s,const char* p2);
int find_pos_sub_str(const struct sub_str s,const struct sub_str x);
void free_http_request(struct http_request *p);

struct sub_str get_line_request(const struct sub_str s);
struct sub_str get_head_request(const struct sub_str s,const struct sub_str l);
struct sub_str get_body_request(const struct sub_str s,const struct sub_str h);

struct http_request* parse_line_request(const struct sub_str l,struct http_request* hr);
struct http_request* parse_head_request(const struct sub_str h,struct http_request* hr);

struct http_request* parse_http_request1(const struct sub_str s,struct http_request *hr);

struct sub_str get_line_request(const struct sub_str s)
{
  struct sub_str r = s;
  r.l = find_pos_sub_str(s,S_S("\r\n"));
  return (r.l>0)?r:S_S(NULL);
};
struct sub_str get_head_request(const struct sub_str s,const struct sub_str l)
{
  struct sub_str h = S_SL(s.s+l.l+2,s.l-l.l-2);
  struct sub_str r = h;
  r.l = find_pos_sub_str(h,S_S("\r\n\r\n"));
  return (r.l>0)?S_SL(r.s,r.l+2):S_S(NULL);
};
struct sub_str get_body_request(const struct sub_str s,const struct sub_str h)
{
  return (((h.s+h.l+4)-s.s)<=s.l)?S_SL(h.s+h.l+4,(h.s+h.l+4)-s.s):S_S(NULL);
};
void print_sub_str(const struct sub_str s)
{
  int i=0;
  printf("sub_str size:%d\n",s.l);
  while((s.s+i)<(s.s+s.l)){
    fprintf(stdout,"%c",*(s.s+i));
    i++;
  }
  printf("\n");
  return;
};
// пропускает пробелы, Иначе NULL
static inline struct sub_str skip_blanks(const struct sub_str s)
{
  int i=0;
  while((s.s+i)!=(s.s+s.l) && isspace((int)*(s.s+i)))
  i++;
  return S_SL(s.s+i,s.l-i);
}
// пропускает НЕ пробелы
static inline struct sub_str skip_nblanks(const struct sub_str s)
{
  int i=0;
  while((s.s+i)!=(s.s+s.l) && !isspace(*(s.s+i)))
  i++;
  return S_SL(s.s+i,s.l-i);
}
int str_case_cmp(const struct sub_str s,const char* p2)
{
  int i=0;
  char *end = (char*)p2+strlen(p2);
  while( ((s.s+i)!=(s.s+s.l)) && ((p2+i)!=end) ){
    if(toupper(*(s.s+i)) != toupper(*(p2+i)))return 0;
    i++;
  };
  return (i==end-p2)?1:0;
};
int find_pos_sub_str(const struct sub_str s,const struct sub_str x)
{
  //print_sub_str(s);
  //print_sub_str(x);
  int i=0,j=0,r=-1;
  while((s.s+i)!=(s.s+s.l)){
    if( toupper(*(s.s+i)) == toupper(*(x.s+j)) ){
      if(j == 0)r=i;
      if((j+1) == x.l)return r;
      j++;
    }else{
      r=-1;
      j=0;
    }
    i++;
  }
  return -1;
};
void print_info_http_reuest(struct http_request *hr)
{
  if(!hr)return;
  switch(hr->method){
  printf("Method: ");
  case GET:
    printf("GET\n");
    break;
  case POST:
    printf("POST\n");
    break;
  case HEAD:
    printf("HEAD\n");
    break;
  }
  if(hr->uri)printf("URI: %s\n",hr->uri);
  if(hr->get_query)printf("Query : %s\n",hr->get_query);
  if(hr->host)printf("HOST: %s\n",hr->host);
  if(hr->user_agent)printf("AGENT: %s\n",hr->user_agent);
  if(hr->cookie)printf("COOKIE: %s\n",hr->cookie);
  if(hr->referer)printf("REFER: %s\n",hr->referer);
  if(hr->accept)printf("Accept: %s\n",hr->accept);
  if(hr->accept_language)printf("Accept-Language: %s\n",hr->accept_language);
  if(hr->accept_encoding)printf("Accept-Encoding: %s\n",hr->accept_encoding);
  if(hr->accept_charset)printf("Accept-Charset: %s\n",hr->accept_charset);
  if(hr->content_type){
  	printf("Content-Type: %s\n",hr->content_type);
  	printf("Content-Length: %d\n",hr->content_length);
  }
}

void *mmap_file(const char *filename,int *size)
{
  int fd;
  struct stat buf;
  void *out=NULL;

  fd=open(filename,O_RDONLY);
  fstat(fd,&buf);
  out=mmap(NULL,buf.st_size,PROT_READ,MAP_PRIVATE,fd,0);
  *size=buf.st_size;
  close(fd);

  return out;
}

void munmap_file(void *buf,int size)
{
  if(!buf || !size)
  return;
  munmap(buf,size);
  return;
}
void free_http_request(struct http_request *p)
{
  if(!p)
    return;
  
  if(p->uri)  free(p->uri);
  if(p->host)  free(p->host);
  if(p->user_agent) free(p->user_agent);
  if(p->accept) free(p->accept);
  if(p->accept_language) free(p->accept_language);
  if(p->accept_encoding) free(p->accept_encoding);
  if(p->accept_charset) free(p->accept_charset);
  if(p->referer) free(p->referer);
  if(p->cookie) free(p->cookie);
  if(p->get_query) free(p->get_query);
  if(p->content_type) free(p->content_type);
  #ifdef MODULAS
  if(p->real_path) free(p->real_path);
  #endif
  free(p);
  
  return;
}

struct http_request* parse_line_request(const struct sub_str l,struct http_request* hr)
{
  //print_sub_str(l);
  if(str_case_cmp(l,"GET")){
    hr->method=GET;
  }else if(str_case_cmp(l,"POST")){
    hr->method=POST;
  }else if(str_case_cmp(l,"HEAD")){
    hr->method=HEAD;
  }else if(str_case_cmp(l,"OPTIONS")){
  }else if(str_case_cmp(l,"PUT")){
  }else if(str_case_cmp(l,"DELETE")){
  }
  if(hr->method==NOT)return hr;
  struct sub_str u = skip_blanks( skip_nblanks(l) ) ;
  struct sub_str p = skip_nblanks(u);
  u.l=p.s-u.s;
  p = skip_blanks(p);
  if(u.l){
    int pos = find_pos_sub_str(u,S_S("?"));
    if(pos != -1){
      vstrfil(hr->uri,pos-1,u.s);
      vstrfil(hr->get_query,u.l-pos-1,u.s+pos+1);
    }else{
      vstrfil(hr->uri,u.l,u.s);
    };
  }
  return hr;
};

struct http_request* parse_head_request(const struct sub_str h,struct http_request* hr)
{
  int pos = 0;
  struct sub_str head = h;
  for( ;(pos = find_pos_sub_str(head,S_S("\r\n")) )!=-1 ; head=S_SL(head.s+pos+2,head.l-pos-2)) {
    struct sub_str tmp = S_SL(head.s,pos);
//	print_sub_str(tmp);
    if(!hr->host && str_case_cmp(tmp,"Host: ")){
      struct sub_str t = skip_blanks( skip_nblanks(tmp) );
      vstrfil(hr->host,t.l,t.s);
	  continue;
    }else if(!hr->user_agent && str_case_cmp(tmp,"User-Agent: ")){
      struct sub_str t = skip_blanks( skip_nblanks(tmp) );
      vstrfil(hr->user_agent,t.l,t.s);
	  continue;
    }else if(!hr->accept && str_case_cmp(tmp,"Accept: ")){
      struct sub_str t = skip_blanks( skip_nblanks(tmp) );
      vstrfil(hr->accept,t.l,t.s);
	  continue;
    }else if(!hr->accept_language && str_case_cmp(tmp,"Accept-Language: ")){
      struct sub_str t = skip_blanks( skip_nblanks(tmp) );
      vstrfil(hr->accept_language,t.l,t.s);
	  continue;
    }else if(!hr->accept_encoding && str_case_cmp(tmp,"Accept-Encoding: ")){
      struct sub_str t = skip_blanks( skip_nblanks(tmp) );
      vstrfil(hr->accept_encoding,t.l,t.s);
	  continue;
    }else if(!hr->accept_charset && str_case_cmp(tmp,"Accept-Charset: ")){
      struct sub_str t = skip_blanks( skip_nblanks(tmp) );
      vstrfil(hr->accept_charset,t.l,t.s);
	  continue;
    }else if(!hr->keep_alive && str_case_cmp(tmp,"Keep-Alive: ")){
      struct sub_str t = skip_blanks( skip_nblanks(tmp) );
      hr->keep_alive = atol(t.s);
	  continue;
    }else if(!hr->cookie && str_case_cmp(tmp,"Cookie: ")){
      struct sub_str t = skip_blanks( skip_nblanks(tmp) );
      vstrfil(hr->cookie,t.l,t.s);
	  continue;
    }else if(!hr->referer && str_case_cmp(tmp,"Referer: ")){
      struct sub_str t = skip_blanks( skip_nblanks(tmp) );
      vstrfil(hr->referer,t.l,t.s);
	  continue;
    }else if(!hr->content_type && str_case_cmp(tmp,"Content-Type: ")){
      struct sub_str t = skip_blanks( skip_nblanks(tmp) );
      vstrfil(hr->content_type,t.l,t.s);
	  continue;
    }else if(str_case_cmp(tmp,"Content-Length:")){
      struct sub_str t = skip_blanks( skip_nblanks(tmp) );
	  hr->content_length = atol(t.s);
	  continue;
    }
//	head=S_SL(head.s+pos+2,head.l-pos-2);
  }
  return hr;
};

struct http_request* parse_http_request1(const struct sub_str s,struct http_request *hr)
{
  struct sub_str line={0,0},head={0,0},body={0,0};
  line = get_line_request(s);

  hr = parse_line_request(line,hr);
  
  head = get_head_request(s,line);
  if(hr->method!=NOT && head.l>0){
//  print_sub_str(head);
    hr = parse_head_request(head,hr);
  }
  
  if(hr->method==POST && head.l>0){
    body = get_body_request(s,head);
//	printf("%d\n",body.l);
 //   print_sub_str(body);
  }
  return hr;
};
/**/
#ifdef MAIN

int main(int argc,char *argv[]){
  if(argc<2)return 0;
  printf("\nopen: %s\n",argv[1]);
  
  struct sub_str s;
  s.s=(char*)mmap_file(argv[1],&s.l);
  
  struct http_request *hr=NULL;
  hr=malloc(sizeof(struct http_request));
  memset(hr,0,sizeof(struct http_request));

  hr = parse_http_request1(s,hr);
  print_info_http_reuest(hr);

  free_http_request(hr);
  munmap_file(s.s,s.l);

  printf("\n");
  return 1;
};
#endif

