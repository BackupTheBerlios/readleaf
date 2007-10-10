/*
 * RedLeaf main entry
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
  main_process(argc,argv);
  /*  int size;
  char *buf=(char*)mmap_file("/home/kaanoken/works/redleaf/src/example.conf",&size);
  load_configuration(buf,size);*/

  return 0;
}
