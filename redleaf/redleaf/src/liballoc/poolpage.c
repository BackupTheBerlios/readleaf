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

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>

#include "../../config.h"

#include <libdata/usrtc.h>
#include <liballoc/poolpage.h>

/*external, public lists*/
extern usrtc_t *global_ipages_used_4k;
extern usrtc_t *global_ipages_free_4k;
extern usrtc_t *global_ipages_used_8k;
extern usrtc_t *global_ipages_free_8k;
extern usrtc_t *global_ipages_used_16k;
extern usrtc_t *global_ipages_free_16k;

/*local(per child)*/
static usrtc_t ipages_used_4k;
static usrtc_t ipages_free_4k;
static usrtc_t ipages_used_8k;
static usrtc_t ipages_free_8k;
static usrtc_t ipages_used_16k;
static usrtc_t ipages_free_16k;


void ipage_alloc(ipage_t *page,uint8_t flags)
{
  size_t size=(8 << 9);

  /*determine size*/
  if(flags & IPG_8K)
    size=(8 << 10);
  else if(flags & IPG_16K)
    size=(8 << 11);
  page->flags |= flags;
  page->flags &= ~IPG_USE;
  if(page->flags & IPG_GLOBAL)
    page->addr=mmap(NULL,size,PROT_READ | PROT_WRITE,MAP_ANON | MAP_SHARED,
		    -1,(off_t)0);
  else 
    page->addr=mmap(0,size,PROT_READ | PROT_WRITE,MAP_PRIVATE | MAP_ANON,
		    -1,(off_t)0);
  if(page->flags & IPG_SECURE)
    memset(page->addr,'\0',size);

  return;
}

void ipage_free(ipage_t *page) 
{
  size_t size=(8 << 9);

  if(page->flags & IPG_8K)
    size=(8 << 10);
  else if(page->flags & IPG_16K)
    size=(8 << 11);
  if(page->flags & IPG_SECURE)
    memset(page->addr,'\0',size);
  page->flags=0x0;
  munmap(page->addr,size);
  page->addr=NULL;

  return;
}

/* push page to used list,
 * doesn't create or alloc 
 * new page from system.
 */
void ipage_push(ipage_t *page)
{
  return;
}

/* pop from used list to free list, 
 * masking it free.
 */
void ipage_pop(ipage_t *page)
{
  return;
}
