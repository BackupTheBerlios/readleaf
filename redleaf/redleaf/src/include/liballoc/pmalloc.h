/*
 * pmalloc for redleafd (related to internal allocator)
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

#ifndef __PMALLOC_H__
#define __PMALLOC_H__

#include <sys/types.h>

#define P_SHARE   (1 << 0)
#define P_SECURE  (1 << 1)

void *pmalloc(size_t size,int flags);
void pfree(void *addr);

/*for shared*/
int pwait_lock(void *addr);
int plock(void *addr);
int punlock(void *addr);

#endif
