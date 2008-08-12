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

#include "../config.h"

#include <http.h>
#include <serv.h>
#include <conf.h>
#include <page.h>
#include <misc.h>
#include <liballoc/memmap.h>

#ifdef MODULAS
#include <modula.h>
#endif

#define CONF_NAME_  "example.conf"

static void _print_version(void)
{
  fprintf(stdout,"RedLeaf httpd version: %d.%d-%s\n",
	  VER_MAJOR,VER_MINOR,VER_SUFFIX);
  fprintf(stdout,"Development tree: %s\n\n",VER_TREE);
  return;
}

int main(int argc, char **argv)
{
  int size,cfsize=0;
  char *buf=NULL;
  char *cnfnm=NULL;
  char *dd=NULL;

  if(argc>=2) {
    dd=argv[1];
    if(*dd=='-' && !strcmp(dd,"-c")) {
      if(argv[2]==NULL) {
	fprintf(stderr,"No parameter given for option.\nExiting.\n");
	return 0;
      }
      cnfnm=argv[2];
    } else if(*dd=='-' && !strcmp(dd,"-v")) {
      _print_version();
      return 0;
    } else {
      fprintf(stderr,"Unknown option or argument `%s'.\nExiting.\n",dd);
      return 0;
    }
  } else {
    cfsize=strlen(CONF_NAME_)+strlen(PREFIX_PATH)+strlen(ETC_PATH)+1;
    cnfnm=rl_malloc(cfsize*sizeof(char));
    if(!cnfnm) {
      fprintf(stderr,"Not enough memory.\nExiting ...\n");
      return -1;
    }
    snprintf(cnfnm,cfsize,"%s%s%s",PREFIX_PATH,ETC_PATH,CONF_NAME_);
    printf("cn: %s\n",cnfnm);
  }

//  mem_proto_init_default(0);

  buf=(char*)mmap_file(cnfnm,&size);
  load_configuration(buf,size);
  init_page_t_cache();
#ifdef MODULAS
  init_modulas_subsystem();
#endif
  main_process(argc,argv);

  munmap_file((void *)buf,size);
  if(cfsize)
    rl_free(cnfnm);

  return 0;
}
