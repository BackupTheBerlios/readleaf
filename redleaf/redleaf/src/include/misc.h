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

#ifndef __MISC_H__
#define __MISC_H__

/*TODO: add check*/
void *mmap_file(const char *filename,int *size);
void munmap_file(void *buf,int size);

char *get_rfc1123date(time_t t);

int norm_slash_uri(char *uri);

/*wrappers for memory related functions*/
void *rl_malloc(size_t size);
void *rl_realloc(void *p,size_t size);
void *rl_calloc(size_t n,size_t size);
void *rl_free(void *p);

#endif

