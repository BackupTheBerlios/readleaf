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

/*TODO:
 * look&feel:
 *  - add types,sizes to the entries
 *  - add better sorting (directories first)
 * functionality:
 *  - add sorting with other methods
 *  - checking
 *  - add url decoding
 */

#define LSHEAD  "<html><head><title>%s contents:</title></head><body>\
<h1>%s directory contents:</h1><hr>\n"
#define LSBOTTOM  "<hr>Redleaf v0.1a<br></body></html>"
#define LSENTRY  "<a href=\"%s/%s\">%s</a><br>\n"

/*local used variables*/
static int total_files=0;

/*local functions prototypes*/
static int qsort_cmp_by_name(const void *a,const void *b);
static char **read_dir_list(const char *path);
static void free_dir_list(char **list);

char *read_dir_contents(const char *filename,const char *uri)
{
  char **dlist=read_dir_list(filename);
  char tbuf[384];
  unsigned int len=strlen(LSHEAD)+strlen(LSBOTTOM)+strlen(uri)*2+1;
  char *outbuf=NULL;
  char *_uri=uri; 
  _uri+=sizeof(char);
  int i;

  len+=(strlen(LSENTRY)+512)*total_files;
  outbuf=calloc(1,len);
  if(!outbuf) {
    fprintf(stderr,"Error allocating memory.\n read_dir_contents()\n");
    goto func_exit;
  }
  sprintf(outbuf,LSHEAD,uri,uri);
  for(i=0;i<=total_files;i++) {
    sprintf(tbuf,LSENTRY,_uri,dlist[i],dlist[i]);
    outbuf=strcat(outbuf,tbuf);
  }
  outbuf=strcat(outbuf,LSBOTTOM);


 func_exit:
  free_dir_list(dlist);

  return outbuf;
}

static int qsort_cmp_by_name(const void *a,const void *b)
{
  return strcmp(*(char **)a, *(char **)b);
}

static char **read_dir_list(const char *path)
{
  char **list=malloc(sizeof(char)*256);
  DIR *dir;
  struct dirent *entry=malloc(sizeof(struct dirent));

  total_files=-1;
  dir=opendir(path);
  while((entry=readdir(dir))) {
    if(!strcmp(entry->d_name,"."))
      continue;
    total_files++;
    if(total_files/256>=1 && total_files%256==0)
      list=realloc(list,sizeof(char)*(((total_files/256)+1)*256));
    list[total_files]=strdup(entry->d_name);
  }
  qsort(list,total_files-1,sizeof(char *),qsort_cmp_by_name);
  list[total_files+1]=NULL;
  closedir(dir);
  free(entry);

  return list;
}

static void free_dir_list(char **list)
{
  int i=0;
  while(list[i]){
    free(list[i]); i++;
  }
  free(list);
}

