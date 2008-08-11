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
  NOT_IMPLEMENTED,
};

inline int cgi_modula_check_capatibilies(int op_code)
{
  return op_cap_codes[op_code];
}

int cgi_modula_init()
{
}

int cgi_modula_shootout()
{
}



static int _run_script(const char *path,const char *args,char *output,char *errmess) 
{
  pid_t pid;
  int o=0;
  int sout[2],serr[2];
  int fo,fe;
  
  if(pipe(sout)==-1) {
    fprintf(stderr,"Error opening pipe for out.\n");
    return -1;
  }
  if(pipe(serr)==-1) {
    fprintf(stderr,"Error opening pipe for err.\n");
    return -1;
  }
  pid=fork();
  if(pid==-1) {
    fprintf(stderr,"Error creating fork()\n");
    return -1;
  }

  if(pid==0) { /*child*/
    int i,u;
    close(sout[0]); /*stdout*/
    close(1);
    i=dup(sout[1]);
    close(sout[1]);

    close(serr[0]); /*stderr*/
    close(2);
    u=dup(serr[1]);
    close(serr[1]);

    if((o=execl(path,(const char*)args,NULL))==-1)
      perror("execl");

  } else { /*parent*/
    char t,y,*oo,*ee;
    int ii=0;
    if(!output || !errmess) {
      fprintf(stderr,"Error allocating memory.\n");
      return -1;
    }
    oo=output;
    ee=errmess;

    close(sout[1]);
    close(serr[1]);
    while(read(sout[0],&t,1)>0) {
      *oo=t; *oo++; ii++;
      if(ii/1024>=1 && ii%1024==0) {
	output=realloc(output,1024*(ii/1024));
      }
    }
    ii=0;
    while(read(serr[0],&y,1)>0) {
      *ee=y; *ee++;
      if(ii/1024>=1 && ii%1024==0) {
	errmess=realloc(output,1024*(ii/1024));
      }
    }
    *ee='\0';*oo='\0';
    close(sout[0]);
    close(serr[0]);

  }
  
  return o;
}

