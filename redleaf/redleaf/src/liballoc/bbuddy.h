/*
 * Originally written by Tirra <tirra.newly@gmail.com>
 * (c) 2008
 *
 * bbuddy.h: functions for allocating within buddy list,
 *           structures used for this.
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

#ifndef __BBUDDY_H__
#define __BBUDDY_H__

typedef struct __reg_desc {
  u_int32_t n[4]; /* negative bitmap */
  u_int32_t p[4]; /* positive bitmap */
  u_int16_t size;
  void *__reg_start;
#ifdef __S1_INFO__
  u_int16_t free;
#endif
#ifdef __D_INFO__
  u_int16_t allocated;
  u_int16_t wasted;
#endif
} reg_desc_t;

/* functions prototypes */
/**
 * __init_reg_desc() initializate region descriptor
 * reg_desc_t*:  pointer to the descriptor
 * void*: pointer to the start of the region
 * u_int16_t: size of the region
 *
 **/
int __init_reg_desc(reg_desc_t *,void *,u_int16_t);

/**
 * __blalloc_ireg() allocate a block within region
 *
 **/
void *__blalloc_ireg(reg_desc_t *,u_int16_t);

/**
 * __blrealloc_ireg() realloc given block within given region
 * the returning pointer can be different than given.
 *
 **/
void *__blrealloc_ireg(reg_desc_t *,void *,u_int16_t);

/**
 * __blfree_ireg() free a block within region
 *
 **/
void __blfree_ireg(reg_desc_t *,void *);

#endif /*__BBUDDY_H__*/

