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
#include <page.h>
#include <misc.h>

#include "../config.h"

#define CONF_NAME_  "example.conf"

int main(int argc, char **argv)
{
  int size,cfsize;
  char *buf=NULL;
  char *cnfnm=NULL;
  char *dd=NULL;

  if(argc>2) {
    dd=argv[1];
    if(*dd=='-' && !strcmp(dd,"-c"))
      cnfnm=argv[2];
  } else {
    cfsize=strlen(CONF_NAME_)+strlen(PREFIX_PATH)+strlen(ETC_PATH)+1;
    cnfnm=malloc(cfsize*sizeof(char));
    if(!cnfnm) {
      fprintf(stderr,"Not enough memory.\nExiting ...\n");
      return -1;
    }
    snprintf(cnfnm,cfsize,"%s%s%s",PREFIX_PATH,ETC_PATH,CONF_NAME_);
  }

  buf=(char*)mmap_file(cnfnm,&size);
  load_configuration(buf,size);
  init_page_t_cache();
  main_process(argc,argv);

  munmap_file((void *)buf,size);
  free(cnfnm);

  return 0;
}
