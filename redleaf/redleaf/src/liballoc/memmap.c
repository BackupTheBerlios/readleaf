/*
 * memory mapping functions for redleafd memory manager
 *
 * Copyright (C) 2006, 2007, 2008 RedLeaf devteam org.
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
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>

#include <liballoc/bbuddy.h>
#include <liballoc/memmap.h>


/* mem_area_alloc() - maps a private mem area */
u_int8_t mem_area_alloc(struct mem_area_t *p,size_t size,bbuddy_t *map)
{
  void *v=mmap(NULL,size,PROT_READ | PROT_WRITE,MAP_PRIVATE,-1,(off_t)0);

  if(v==MAP_FAILED) {
    fprintf(stderr,"Mapping failed.\n");
    perror("mmap:");
    return 1;
  }
  p->size=size;
  p->map=map;
  p->area=v;
  p->lock=0;

  return 0;
}

/* mem_area_public_alloc() - maps a shared mem area */
u_int8_t mem_area_public_alloc(struct mem_area_t *p,size_t size,bbuddy_t *map,int lock_flag)
{
  void *v=mmap(NULL,size,PROT_READ | PROT_WRITE,MAP_ANON | MAP_SHARED,-1,(off_t)0);

  if(v==MAP_FAILED) {
    fprintf(stderr,"Mapping failed.\n");
    perror("mmap:");
    return 1;
  }
  p->size=size;
  p->map=map;
  p->area=v;
  p->lock=lock_flag;

  return 0;
}

/* mem_area_release() - release mem area */
u_int8_t mem_area_release(struct mem_area_t *p)
{
  mem_area_wait_lock(p);

  mem_area_lock(p);

  if(!p || !p->area) {
    fprintf(stderr,"Trying to release not mapped mem area.\n");
    return 1;
  }

  if(munmap(p->area,p->size)) {
    fprintf(stderr,"Unmapping failed.\n");
    perror("munmap:");
    return 1;
  }

  return 0;
}

inline u_int8_t mem_area_lock(struct mem_area_t *p)
{
  if(!p) {
    fprintf(stderr,"Memory area pointer is not valid, cannot lock.\n");
    return 1;
  }
  if(!p->lock) {
    fprintf(stderr,"Memory area cannot be locked in case of private state.\n");
    return 0;
  }
  p->lock++;

  return 0;
}

inline u_int8_t mem_area_unlock(struct mem_area_t *p)
{
  if(!p) {
    fprintf(stderr,"Memory area pointer is not valid, cannot unlock.\n");
    return 1;
  }
  if(!p->lock || p->lock<2) {
    fprintf(stderr,"Memory area isn't locked, or not lockable.\n");
    return 0;
  }

  p->lock--;

  return 0;
}


