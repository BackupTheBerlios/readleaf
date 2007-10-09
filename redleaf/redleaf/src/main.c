#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <http.h>
#include <serv.h>
#include <conf.h>

int main(int argc, char **argv)
{
  /*  char *treq=malloc(4096);
  struct http_request *h;
  int i=0;
  snprintf(treq,4096,"GET /index.cgi HTTP/1.1\nASD: asdsadsa\ndf: -h\n\n");
  h=parse_http_request(treq);
  printf("Method: %s\n"\
	 "Location: %s\n"\
	 "Version: %s\n",h->method,h->location,
	 h->pver);
  for(i=0;i<h->vlist;i++)
  printf("VAR(%s)=%s\n",h->vars[i],h->values[i]);*/
  //  main_process(argc,argv);
  int size;
  char *buf=(char*)mmap_file("/home/kaanoken/works/redleaf/src/example.conf",&size);
  load_configuration(buf,size);

  return 0;
}
