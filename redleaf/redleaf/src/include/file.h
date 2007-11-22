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

#ifndef __FILE_H__
#define __FILE_H__

#include <sys/types.h>
#include <unistd.h>

struct file_session_t {
  char *filename;  /*file name*/
  int fd;          /*descriptor*/
  off_t cur_off;   /*current offset*/
  size_t file_len; /*file length*/
  void *buf;       /*temproary buffer*/
  size_t buf_len;  /*length of the buffer*/
};

struct file_session_t *create_file_session(const char *filename,size_t buf_len);
void destroy_file_session(struct file_session_t *p);
void *read_file_session(struct file_session_t *p,size_t *chunk_len);
void updoffset_file_session(struct file_session_t *p,off_t offset);

#endif
