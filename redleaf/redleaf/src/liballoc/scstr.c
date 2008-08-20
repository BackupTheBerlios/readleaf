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
#include <libdata/usrtc.h> 

#define DEF_CSSTRAMOUNT  256
#define DEF_ATHRESHOLD   128
#define DEF_ALIGN        DEF_CSSTRAMOUNT/32

/*
 * TODO: 
 * ** rewrite all to be more kindly,
 *    add counter to scstr_t - counter of reallocations, 
 *    and if we're making many remappings - increase chunk size 
 *    (somebody's fucking deal)
 * ** Create a pull of stream to make a choose from it's to avoid
 *    many mmap() calls (stupidity-safe)
 */

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
    fprintf(stderr,"[EE] mmap failed, aborting.\n");
    perror("mmap:");
    return NULL;
  }
  r=(scstr_t*)pp;
  r->size=def_amount;
  r->off_size=odf;
  r->used=0;
  r->amount=def_amount;
  r->pp=(void*)((char*)pp+odf);
  memset(r->pp,'\0',def_amount);

  return r;
}

scstr_t *scstr_add(scstr_t *sc,char *cstr)
{
  int len,i;
  void *n; 
  scstr_t *re;

  if(!sc) {
    fprintf(stderr,"[EE] Given CString stream is nil.\n");
    return NULL;
  }

  len=strlen(cstr);
  if((sc->size)-(sc->used)<len+1) {
    if(len+1>sc->amount) { /*oops, let's reassign new stream in case of somebody's stupidity*/
      scstr_t *nsc=scstr_new((len+1)*4);
      /*ok, let's copy old content*/
      nsc->used=sc->used;
      strcpy((char*)nsc->pp,(const char*)sc->pp);
      scstr_remove(sc);
      return scstr_add(nsc,cstr);
    }
    /*let's reallocate*/
    i=sc->size/sc->amount;
    n=mmap(0,((i+1)*sc->amount)+sc->off_size,PROT_READ | PROT_WRITE,MAP_PRIVATE | MAP_ANON,-1,(off_t)0);
    if(n==MAP_FAILED) {
      fprintf(stderr,"[EE] mmap failed, aborting.\n");
      perror("mmap:");
      return sc;
    }
    memset(n,'\0',((i+1)*sc->amount)+sc->off_size);
    memcpy(n,(const void*)sc,sc->off_size+sc->used); /*copy all contents*/
    re=(scstr_t*)n;
    re->pp=(void*)((char*)n+sc->off_size); /*reassign new pointer*/
    scstr_remove(sc); /*unmap all data*/
    strcat((char*)re->pp,cstr); /*actual cat*/
    re->used+=len;
    return re;
  } else { /*simply concatenate given string*/
    strcat((char*)sc->pp,cstr);
    sc->used+=len;
  }

  return sc;
}

scstr_t *scstr_addn(scstr_t *sc,char *cstr,int len)
{
  char ss=cstr[len+1]; /*ugly hack,we'll change cstring for time being*/

  cstr[len+1]='\0'; /*limit cstring*/
  sc=scstr_add(sc,cstr); /*do actual add*/
  cstr[len+1]=ss; /*restore*/

  return sc;
}

size_t scstr_len(scstr_t *sc)
{
  if(!sc) {
    fprintf(stderr,"[EE] Given CString stream is nil.\n");
    return 0;
  }
  else return sc->used; /*you know*/
}

int scstr_clear(scstr_t *sc)
{
  if(!sc) {
    fprintf(stderr,"[EE] Given CString stream is nil.\n");
    return -1;
  }

  memset(sc->pp,'\0',sc->size); /*simply set alls to zero*/
  sc->used=0;

  return 0;
}

int scstr_remove(scstr_t *sc)
{
  if(!sc) {
    fprintf(stderr,"[EE] Given CString stream is nil.\n");
    return -1;
  }

  if(munmap((void*)sc,sc->size+sc->off_size)) {
    fprintf(stderr,"[EE] Unmapping failed.\n");
    perror("munmap:");
    return -1;
  }

  return 0;
}

char *scstr_get(scstr_t *sc)
{
  if(!sc) {
    fprintf(stderr,"[EE] Given CString stream is nil.\n");
    return NULL;
  }

  return (char*)sc->pp; /*return pointer to real data*/
}

