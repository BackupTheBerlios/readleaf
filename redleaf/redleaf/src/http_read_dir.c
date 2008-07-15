/*
 * RedLeaf reading directory application
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

#include <misc.h>

/*TODO: add better sorting (directories first), add url decoding
 * other improvments moved to the module
 */

#define LSHEAD  "<html><head><title>%s contents:</title></head><body>\
<h1>%s directory contents:</h1><hr>\n"
#define LSBOTTOM  "<hr>Redleaf v0.1beta<br></body></html>"
#define LSENTRY  "<a href=\"%s\">%s</a><br>\n"

/*local used variables*/
static int total_files=0;

/*local functions prototypes*/
static int qsort_cmp_by_name(const void *a,const void *b);
static char **read_dir_list(const char *path);
static void free_dir_list(char **list);

char *read_dir_contents(const char *filename,const char *uri)
{
  char **dlist=read_dir_list(filename);
  char *tbuf=rl_malloc(384); memset(tbuf,'\0',384);
  unsigned int len=strlen(LSHEAD)+strlen(LSBOTTOM)+strlen(uri)*2+1;
  char *outbuf=NULL;
  int i;

  len+=(strlen(LSENTRY)+512)*total_files;
  outbuf=rl_malloc(len);
  memset(outbuf,'\0',len);

  sprintf(outbuf,LSHEAD,uri,uri);
  for(i=0;i<total_files;i++) {
    if(dlist[i]) {
      sprintf(tbuf,LSENTRY,dlist[i],dlist[i]);
      outbuf=strcat(outbuf,tbuf);
    }
  }
  outbuf=strcat(outbuf,LSBOTTOM);

  free_dir_list(dlist);
  rl_free(tbuf);

  total_files=0;

  return outbuf;
}

static int qsort_cmp_by_name(const void *a,const void *b)
{
  return strcmp(*(char **)a, *(char **)b);
}

static char **read_dir_list(const char *path)
{
  char **list=rl_malloc(sizeof(char)*256);
  DIR *dir;
  struct dirent *entry;

  total_files=0;
  dir=opendir(path);
  while((entry=readdir(dir))) {
    if(!strcmp(entry->d_name,"."))
      continue;
    else {
      if(total_files/256>=1 && total_files%256==0) {
	list=rl_realloc(list,sizeof(char)*(((total_files/256)+1)*256));
      }
      list[total_files]=strdup(entry->d_name);
      total_files++;
    }
  }
  qsort(list,total_files,sizeof(char *),qsort_cmp_by_name);
  list[total_files+1]=NULL;
  closedir(dir);

  return list;
}

static void free_dir_list(char **list)
{
  int i=0;
  while(i<total_files){
    rl_free(list[i]); i++;
  }
  rl_free(list);
}
