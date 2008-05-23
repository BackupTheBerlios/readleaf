/*
 * Originally written by Tirra <tirra.newly@gmail.com>
 * (c) 2008
 *
 * bbuddy.c: functions for allocating within buddy list (implementation),
 *           some useful defines (internally used).
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
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include "bbuddy.h"

#define nil  0x0
#define fil  0xffffffff

/*TODO: optimize for stack size and speed, implement S1 and verbose features*/

/* local functions prototypes */
static int16_t __red_bt(reg_desc_t *r,u_int8_t p,u_int8_t l);
static int16_t __reg_bt(reg_desc_t *r,u_int8_t p);
static int __get_offset(reg_desc_t *r,u_int8_t p);
#ifdef _DEBUG_
static void __print_bitmap(u_int8_t size,u_int64_t o);
static void __print_region(reg_desc_t *ss);
static void __print_region_p(reg_desc_t *ss);
#endif

/* general implementation */
int __init_reg_desc(reg_desc_t *ss,void *p,u_int16_t size) /*init region descriptor*/
{
  int8_t i;

  if(!ss || !p) {
#ifdef _DEBUG_
    fprintf(stderr,"Given pointers are invalid (nil)\n");
#endif
    return 1;
  }
  if(size<512 || size%64) {
#ifdef _DEBUG_
    fprintf(stderr,"Given size `%d' is incorrect.\n",size);
#endif
    return 1;
  }
  ss->size=size;
  ss->__reg_start=p;
  for(i=0;i<4;i++)
    ss->n[i]=nil;
  for(i=0;i<4;i++)
    ss->p[i]=fil;
  /*init first regions*/
  ss->n[0] |= (1 << 0);
  ss->n[0] |= (1 << 1);

  return 0;
}

void *__blalloc_ireg(reg_desc_t *r,u_int16_t byte) /* allocate block within region*/
{
  u_int16_t block_size,i=0;
  u_int32_t offset;

  if(!r) {
#ifdef _DEBUG_
    fprintf(stderr,"__blalloc_ireg: descriptor is nil.\n");
#endif
    return NULL;
  }
  if(!byte)
    return NULL;
  if(byte>r->size) {
#ifdef _DEBUG_
    fprintf(stderr,"__blalloc_ireg: requested size too big for tthis pool.\n");
#endif
    return NULL;
  }
  while(r->size/(1 << i) >= byte && i<7) {
    block_size=1 << i;
    i++;
  }
  if((offset=__get_offset(r,block_size))<0) {
#ifdef _DEBUG_
    fprintf(stderr,"No space avialable for this block.\n");
#endif
    return NULL;
  }

  return r->__reg_start+(block_size*offset);
}

void __blfree_ireg(reg_desc_t *r,void *p) /* free block within given region */
{
  u_int32_t offset;
  u_int8_t i;
  u_int8_t blk=0,off,n,area;

  if(!r) {
#ifdef _DEBUG_
    fprintf(stderr,"__blfree_ireg: Region descriptor is nil.\n");
#endif
    return;
  }
  if(p>(r->__reg_start+r->size) && p<r->__reg_start) {
#ifdef _DEBUG_
    fprintf(stderr,"__blfree_ireg: Pointer is invalid for this region\n");
#endif
    return;
  }

  /*try to determine */
  offset=p-r->__reg_start;
  //  printf("offset=%d\n",offset);
  for(i=0;i<7;i++) 
    if(!(offset%(r->size/(1 << i)))){
      blk=(1 << i);
      break;
    }
  if(!blk) {
#ifdef _DEBUG_
    fprintf(stderr,"__blfree_ireg: Invalid pointer given.\n");
#endif
    return;
  }
  for(;i<7;i++){
    n=offset/(r->size/blk);
    off=(1 << i);
    area=(1 << i)/32;
    if(off>31) 
      off-=32*area;
    if(n>31) n-=32;
    if((r->p[area] & (1 << (off+n))) && !(r->n[area] & (1 << (off+n)))) {
      //printf("found ! block size = %d, area: %d, pitch: %d\n",r->size/(1 << i),area,off+n);
      while(blk>0) {
	/*check free*/
	r->n[area] |= (1 << (off+n));
	r->p[area] |= (1 << (off+n));
	/*look if we're can split block*/
	if(blk>1) {
	  if((off+n)%2 && r->n[area] & (1 << ((off+n)-1))) /*left buddy*/ {
	    n--;
	    r->n[area] &= ~(1 << (off+n)); /*clean*/
	    r->n[area] &= ~(1 << ((off+n)+1));
	  }
	  else if(!((off+n)%2) && r->n[area] & (1 << ((off+n)+1))) /*right buddy*/ {
	    r->n[area] &= ~(1 << (off+n)); /*clean*/
	    r->n[area] &= ~(1 << ((off+n)+1));
	  }
	  else
	    break;
	} else 
	  break;
	/*okay calculate separated parent*/
	if(area==1) area=0;
	else if(area>1) area=1;
	n=n/2; blk/=2; off=blk;
	//printf("off %d,blk %d, n %d, area %d\n",off,blk,n,area);
      }
      
      break;
    }
    blk*=2;
  }

  return;
}

void *__blrealloc_ireg(reg_desc_t *r,void *p,u_int16_t size) /* reallocate block within region to given size */
{
  int8_t i;
  u_int16_t offset,blk=0;

  if(!r) {
#ifdef _DEBUG_
    fprintf(stderr,"Given region pointer are invalid.\n");
#endif
    return p;
  }
  if(!size) {
    __blfree_ireg(r,p);
    return NULL;
  }
  if(p>(r->__reg_start+r->size) && p<r->__reg_start) {
#ifdef _DEBUG_
    fprintf(stderr,"__blrealloc_ireg: Pointer is invalid for this region\n");
#endif
    return p;
  }
  /*determine*/
  for(i=0;i<7;i++) 
    if(!(offset%(r->size/(1 << i)))) {
      blk=(1 << i);
      break;
    }
  if(!blk) {
#ifdef _DEBUG_
    fprintf(stderr,"__blfree_ireg: Invalid pointer given.\n");
#endif
    return p;
  }
  if(blk>=size)
    return p;
  else {
    __blfree_ireg(r,p);
    return __blalloc_ireg(r,size);
  }

  return p;
}

/* local functions implementation */
static int16_t __red_bt(reg_desc_t *r,u_int8_t p,u_int8_t l)
{
  u_int8_t q,a,i,area=l/(8*(sizeof(u_int32_t))),off=l;

  if(!l) {
#ifdef _DEBUG_
    fprintf(stderr,"No appropriate block size found.\n");
#endif
    return -1;
  }

  if(off>((8*(sizeof(u_int32_t)))-1))    off=0; /*trancate*/

  /*try to find appropriate free block*/
  for(i=0;i<l;i++) {
    if(i == (8*(sizeof(u_int32_t)))) {
      area++;
      off-=(8*(sizeof(u_int32_t)));
    }

    if(r->n[area] & (1 << (off+i))) {
      q=1;
      if(p!=l) { /*separate*/
	r->n[area] &= ~(1 << (off+i)); /*mark that separated && busy*/
	r->p[area] &= ~(1 << (off+i)); /*separated*/
	/*mark below block avialable*/
	if(off*2 == (8*(sizeof(u_int32_t)))) {
	  area++;
	  off=0;
	} 
	if(l*2 == ((8*(sizeof(u_int32_t)))*2)) {
	  off=0; area++;
	  if((2*i) > ((8*(sizeof(u_int32_t)))-1))
	    area++;
	}

	for(a=0;a<2;a++)
	  r->n[area] |= (1 << ((off*2)+(2*i)+a));

	return __red_bt(r,p,l*2);
      } else { /*allocate*/
	r->n[area] &= ~(1 << (off+i)); /*mark that separated && whole used*/
	r->p[area] |= (1 << (off+i)); /*whole used*/
	return i;
      }
    } else q=0;
  }
  
  if(!q)
    return __red_bt(r,p,l/2);
  
  return -1;
}

static int16_t __reg_bt(reg_desc_t *r,u_int8_t p)
{
  return __red_bt(r,p,p);
}

static int __get_offset(reg_desc_t *r,u_int8_t p)
{
  int16_t voffset;

  if((voffset=__reg_bt(r,p))<0) {
#ifdef _DEBUG_
    fprintf(stderr,"There are no blocks avialable for this size;\n");
#endif
    return -1;
  }

  return voffset*(r->size/p);
}

#ifdef _DEBUG_
static void __print_bitmap(u_int8_t size,u_int64_t o)
{
  int8_t i;
  for(i=0;i<size;i++) {
    if(o & (1 << i))
      printf("1");
    else
      printf("0");
  }
  printf("\n");

  return;
}

static void __print_region(reg_desc_t *ss)
{
  printf("area 0: ");
  __print_bitmap(32,ss->n[0]);
  printf("area 0: R122444488888888FFFFFFFFFFFFFFFF\n");
  printf("area 1: ");
  __print_bitmap(32,ss->n[1]);
  printf("area 2: ");
  __print_bitmap(32,ss->n[2]);
  printf("area 3: ");
  __print_bitmap(32,ss->n[3]);

  return;
}

static void __print_region_p(reg_desc_t *ss)
{
  printf("area 0: ");
  __print_bitmap(32,ss->p[0]);
  printf("area 0: R122444488888888FFFFFFFFFFFFFFFF\n");
  printf("area 1: ");
  __print_bitmap(32,ss->p[1]);
  printf("area 2: ");
  __print_bitmap(32,ss->p[2]);
  printf("area 3: ");
  __print_bitmap(32,ss->p[3]);

  return;
}
#endif /*_DEBUG_*/

