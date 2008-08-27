/*
 * page pools for redleafd (related to internal allocator)
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

#ifndef __POOLPAGE_H__
#define __POOLPAGE_H__

#include <sys/types.h>

/*ipage flags*/
#define IPG_4K      (1 << 0)
#define IPG_8K      (1 << 1)
#define IPG_16K     (1 << 2)
#define IPG_GLOBAL  (1 << 3)
#define IPG_SECURE  (1 << 4)
#define IPG_USE     (1 << 5)

typedef u_int8_t uint8_t;
typedef u_int16_t uint16_t;
typedef u_int32_t uint32_t;

typedef struct __ipage_t {
  void *addr; /*address of page*/
  uint8_t flags; /*page flags*/
  uint8_t lock; /*lock, used only if public flag masked*/
  time_t last_accessed; /*for gc-like collecting*/
} ipage_t;

void ipage_alloc(ipage_t *page,uint8_t flags);
void ipage_free(ipage_t *page);

/* push page to used list,
 * doesn't create or alloc 
 * new page from system.
 */
void ipage_push(ipage_t *page);

/* pop from used list to free list, 
 * masking it free.
 */
void ipage_pop(ipage_t *page);

#endif /*__POOLPAGE_H__*/

