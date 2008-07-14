/*
 * RedLeaf support/misc functions
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
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#include <misc.h>

#define DATELEN  64

char *wkday[7]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
char *month[12]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct",
		 "Nov","Dec"};

/*TODO: add check*/
void *mmap_file(const char *filename,int *size)
{
  int fd;
  struct stat buf;
  void *out=NULL;

  fd=open(filename,O_RDONLY); /*TODO: make check*/
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

char *get_rfc1123date(time_t t)
{
  struct tm *tm;
  char *obuf=NULL;

  obuf=rl_malloc(DATELEN);
  tm=localtime(&t);
  if(!obuf)
    return NULL;
  sprintf(obuf, "%s, %02d %s %04d %02d:%02d:%02d GMT",wkday[tm->tm_wday],
	  tm->tm_mday,month[tm->tm_mon],tm->tm_year+1900,tm->tm_hour,tm->tm_min,
	  tm->tm_sec);

  return obuf;
}

int norm_slash_uri(char *uri)
{
  char *bt=uri;
  int i=strlen(uri)-1;

  while(bt[i]=='/' || i>=1) {
    if(bt[i-1]=='/') {
      bt[i]='\0'; i--;
    } else
      break;
  }

  return 0;
}

void *rl_malloc(size_t size)
{
  void *v=malloc(size);

  if(!v) {
    fprintf(stderr,"Error allocating memory for %d bytes.\n",(int)size);
    exit(3);
    return NULL;
  }

  return v;
}

void *rl_realloc(void *p,size_t size)
{
  p=realloc(p,size);

  if(!p) {
    fprintf(stderr,"Error re-allocating memory with new size %d bytes.\n",(int)size);
    exit(3);
    return NULL;
  }

  return p;
}

void *rl_calloc(size_t n,size_t size)
{
  void *v=calloc(n,size);

  if(!v) {
    fprintf(stderr,"Error allocating %d blocks with %d size.\n",(int)n,(int)size);
    exit(3);
    return NULL;
  }

  return v;
}

void *rl_free(void *p)
{
  if(p)    free(p);
  else     fprintf(stderr,"Trying to free nil pointer.\n");

  return NULL;
}

