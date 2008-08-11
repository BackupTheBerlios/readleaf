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

#include <libdata/usrtc.h>
#include <libdata/macro.h>
#include <liballoc/bbuddy.h>
#include <liballoc/memmap.h>

#define AREA_SIZE  (1 << 17)

struct __chunkm_t {
  struct mem_area_t *d;
  struct __chunkm_t *next;
};

static struct __chunkm_t *chead=NULL;

/*locally used functions*/
static void *__mab_alloc(struct mem_area_t *p,size_t size);
static void __mab_free(struct mem_area_t *p,void *m);

void *balloc(size_t size)
{
  struct __chunkm_t *__chead=chead;

  while((__chead->d->size-__chead->d->commit)<size) {
    if(__chead->next==NULL) {
      goto __extend;
      break;
    }
    __chead=__chead->next;
  }

  if(__chead) 
    return __mab_alloc(__chead->d,size);

  if(!chead) {
//    chead=slab_alloc(sizeof(struct __chunkm_t));
    chead->next=NULL;
//    chead->d=slab_alloc(sizeof(struct mem_area_t));
    return __mab_alloc(chead->d,size);
  } else {
  __extend:
//    __chead->next=slab_alloc(sizeof(struct __chunkm_t));
    __chead=__chead->next;
    __chead->next=NULL;
//    __chead->d=slab_alloc(sizeof(struct mem_area_t));
  }

  return __mab_alloc(__chead->d,size);
}

void bfree(void *pp)
{
  return;
}

/* mem_area_alloc() - maps a private mem area */
u_int8_t mem_area_alloc(struct mem_area_t *p,size_t size,bbuddy_t *map)
{
  void *v=mmap(0,size,PROT_READ | PROT_WRITE,MAP_PRIVATE | MAP_ANON,-1,(off_t)0);

  if(v==MAP_FAILED) {
    fprintf(stderr,"Mapping failed.\n");
    perror("mmap:");
    return 1;
  }
  p->size=size;
  p->map=map;
  p->area=v;
  p->lock=0;
  p->ref=0;
  p->commit=0;

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
  p->ref=0;
  p->commit=0;

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

  if(p->ref)
    fprintf(stderr,"Releasing area that maybe used.\n");

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

static void __mab_free(struct mem_area_t *p,void *m)
{
  u_int32_t layer;

  if(bbuddy_block_release(p->map,(m-(p->area))/p->map->pn,&layer)) {
    fprintf(stderr,"Invalid pointer to free.\nExiting.\n");
    exit(3);
  }
  p->commit-=p->size/layer;

  return;
}

static void *__mab_alloc(struct mem_area_t *p,size_t size)
{
  u_int32_t layer,i;

  if(size>p->size)
    return NULL;
  layer=p->size/size;
  for(i=0;(1 << i)<=p->map->pn;i++)    
    if(layer >= (1 << i) && layer < (1 << (i+1)))      break;
  layer=i;
  p->commit+=p->size/layer;
  i=bbuddy_block_alloc(p->map,(1 << layer));

  return (void *)((char*)p->area+(i*(p->size/(1 << layer))));
}
