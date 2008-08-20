/*
 * StreamCString implementation for redleafd (related to internal allocator)
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

#ifndef __CSSTR_H__
#define __CSSTR_H__

#include <sys/types.h>

typedef struct __scstr_t {
  size_t amount;
  int off_size;
  size_t used;
  size_t size;
  void *pp;
} scstr_t;

scstr_t *scstr_new(size_t def_amount);

scstr_t *scstr_add(scstr_t *sc,char *cstr);

scstr_t *scstr_addn(scstr_t *sc,char *cstr,int len);

size_t scstr_len(scstr_t *sc);

int scstr_clear(scstr_t *sc);

int scstr_remove(scstr_t *sc);

#endif

