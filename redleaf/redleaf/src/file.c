/*
 * RedLeaf file_session_t operations
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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>

#include <file.h>
#include <misc.h>

/*TODO: makes checks*/

struct file_session_t *create_file_session(const char *filename,size_t buf_len)
{
  struct file_session_t *p=NULL;
  struct stat st;

  p=rl_malloc(sizeof(struct file_session_t));
  p->filename=strdup(filename);
  if(!p->filename) {
    fprintf(stderr,"Error allocating memory.\n");
    rl_free(p);
    return NULL;
  }
  p->buf=rl_malloc(buf_len);
  if(!p->buf) {
    fprintf(stderr,"Error allocating memory.\n");
    rl_free(p->filename);
    rl_free(p);
    return NULL;
  }
  p->cur_off=0;
  p->buf_len=buf_len;
  p->fd=open(filename,O_RDONLY);
  if(p->fd==-1) {
    fprintf(stderr,"Error opening file `%s'\n",filename);
  _open_fail:
    rl_free(p->filename);
    rl_free(p->buf);
    rl_free(p);
    return NULL;
  }
  if(stat(p->filename,&st)) {
    fprintf(stderr,"Error stat file `%s'\n",filename);
    perror("stat:");
    goto _open_fail;
  }
  p->file_len=st.st_size;

  return p;
}

void destroy_file_session(struct file_session_t *p)
{
  if(p->buf)
    rl_free(p->buf);
  if(p->filename)
    rl_free(p->filename);

  if(close(p->fd)) {
    fprintf(stderr,"Warning: error closing file descriptor.\n");
    perror("close:");
  }

  rl_free(p);

  return;
}

void updoffset_file_session(struct file_session_t *p,off_t offset)
{
  if(lseek(p->fd,offset,SEEK_SET)!=offset)
    perror("lseek:");
  else
    p->cur_off=offset;

  return;
}

void *read_file_session(struct file_session_t *p,size_t *chunk_len)
{
  size_t cl=0;

  if(p->buf_len+p->cur_off > p->file_len)
    *chunk_len=p->file_len-p->cur_off;
  else 
    *chunk_len=p->buf_len;

  if((cl=read(p->fd,p->buf,*chunk_len))==-1) {
    fprintf(stderr,"Error reading from file `%s'\n",p->filename);
    perror("read:");
    return NULL;
  } 

  return p->buf;
}

