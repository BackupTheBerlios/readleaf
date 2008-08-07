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

#ifndef __MEMMAP_H__
#define __MEMMAP_H__

#include <sys/types.h>

#include <liballoc/bbuddy.h>
#include <libdata/usrtc.h>

#define MAP_LOCKABLE     1
#define MAP_NOTLOCKABLE  0

#define MPROTO_PRIV    (1 << 0)
#define MPROTO_LOCK    (1 << 1)
#define MPROTO_SHARED  (1 << 2)
#define MPROTO_SLAB    (1 << 3)

#define mem_area_wait_lock(p)  if(p->lock>MAP_LOCKABLE) while(p->lock!=MAP_LOCKABLE) usleep(100);

struct mem_area_t {
  size_t size;   /*size of area*/
  size_t commit; /*commited memory amount*/
  void *area;    /*mapped area pointer*/
  bbuddy_t *map; /*buddy allocator*/
  u_int8_t lock; /*sync primitive*/
  u_int32_t ref; /*referenced pools count*/
};

struct mem_proto_t {
  usrtc_t *proto; /*structure*/
  pid_t owner;
  u_int8_t lock;
  u_int8_t flags;
};

/* mem_area_alloc() - maps a private mem area */
extern u_int8_t mem_area_alloc(struct mem_area_t *p,size_t size,bbuddy_t *map);

/* mem_area_public_alloc() - maps a shared mem area */
extern u_int8_t mem_area_public_alloc(struct mem_area_t *p,size_t size,
				      bbuddy_t *map,int lock_flag);

/* mem_area_release() - release mem area */
extern u_int8_t mem_area_release(struct mem_area_t *p);

/*syncing functions*/
extern inline u_int8_t mem_area_lock(struct mem_area_t *p);
extern inline u_int8_t mem_area_unlock(struct mem_area_t *p);

#endif /*__MEMMAP_H__*/

