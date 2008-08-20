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

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>

#include <liballoc/scstr.h>

#define DEF_CSSTRAMOUNT  256
#define DEF_ATHRESHOLD   128
#define DEF_ALIGN        DEF_CSSTRAMOUNT/32

scstr_t *scstr_new(size_t def_amount)
{
  size_t odf=sizeof(scstr_t);
  scstr_t *r;
  void *pp;
  int align=def_amount/32;

  if(def_amount<DEF_ATHRESHOLD) {
    def_amount=DEF_CSSTRAMOUNT;
    align=DEF_ALIGN;
  }

  if(odf%align)
    odf+=align-(odf%align);

  pp=mmap(0,def_amount+odf,PROT_READ | PROT_WRITE,MAP_PRIVATE | MAP_ANON,-1,(off_t)0);
  if(pp==MAP_FAILED) {
    fprintf(stderr,"mmap failed, aborting.\n");
    perror("mmap:");
    return NULL;
  }
  r=(scstr_t*)pp;
  r->size=def_amount;
  r->off_size=odf;
  r->used=0;
  r->amount=def_amount;
  r->pp=(void*)((char*)pp+odf);

  return r;
}

scstr_t *scstr_add(scstr_t *sc,char *cstr)
{
  return sc;
}

scstr_t *scstr_addn(scstr_t *sc,char *cstr,int len)
{
  return sc;
}

size_t scstr_len(scstr_t *sc)
{
  if(!sc) {
    fprintf(stderr,"Given CString stream is nil.\n");
    return 0;
  }
  else return sc->used;
}

int scstr_clear(scstr_t *sc)
{
  return 0;
}

int scstr_remove(scstr_t *sc)
{
  return 0;
}

