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

/*local functions prototypes*/
static int qsort_cmp_by_name(const void *a,const void *b);
static char **read_dir_list(const char *path);
static void free_dir_list(char **list);

int write_html_dir_list(int fd,const char *path,const char *uri)
{
  int i=0,o=0;
  char **list=read_dir_list(path);

  while(list[i]) {
    o+=write(fd,"<a href=\"",strlen("<a href=\""));
    if(strcmp(uri,"/"))
      o+=write(fd,uri,strlen(uri));
    o+=write(fd,"/",strlen("/"));
    o+=write(fd,list[i],strlen(list[i]));
    o+=write(fd,"\">",strlen("\">"));
    o+=write(fd,list[i],strlen(list[i]));
    o+=write(fd,"</a><br>",strlen("</a><br>"));
    i++;
  }

  free_dir_list(list);

  return o;
}

static int qsort_cmp_by_name(const void *a,const void *b)
{
  return strcmp(*(char **)a, *(char **)b);
}

static char **read_dir_list(const char *path)
{
  int i=-1;
  char **list=malloc(sizeof(char)*256);
  DIR *dir;
  struct dirent *entry=malloc(sizeof(struct dirent));

  dir=opendir(path);
  while((entry=readdir(dir))) {
    i++;
    if(i/256>=1 && i%256==0)
      list=realloc(list,sizeof(char)*(((i/256)+1)*256));
    list[i]=strdup(entry->d_name);
  }
  qsort(list,i,sizeof(char *),qsort_cmp_by_name);
  i++;list[i]=NULL;
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

