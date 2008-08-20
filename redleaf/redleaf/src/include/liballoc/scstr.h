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

#ifndef __SCSTR_H__
#define __SCSTR_H__

#include <sys/types.h>

/*
 * TODO: 
 * ** rewrite all to be more kindly,
 *    add counter to scstr_t - counter of reallocations, 
 *    and if we're making many remappings - increase chunk size 
 *    (somebody's fucking deal)
 * ** Create a pull of stream to make a choose from it's to avoid
 *    many mmap() calls (stupidity-safe)
 */

typedef struct __scstr_t {
  size_t amount; /*chunk size*/
  int off_size; /*overdraft size*/
  size_t used; /*really used*/
  size_t size; /*real size*/
  void *pp; /*real pointer to the data*/
} scstr_t;

/*======pure CString stream operations=====*/
/**
 * scstr_new(): create new CString stream
 * (def_amount: size_t) default chunk size
 * return: pointer to a newly allocated stream
 */
scstr_t *scstr_new(size_t def_amount);
/**
 * scstr_add(): adds cstring to stream
 * (sc: *scstr_t) pointer to existing CString stream
 * (scstr: *char) pointer to cstring
 * return: pointer to stream
 * >NOTE: always assign your pointer to this
 *        function return, because pointer to
 *        stream can be changed
 */
scstr_t *scstr_add(scstr_t *sc,char *cstr);
/**
 * scstr_addn(): adds cstring to stream (with lenght limit)
 * (sc: *scstr_t) pointer to existing CString stream
 * (scstr: *char) pointer to cstring
 * (len: int) lenght to add
 * return: pointer to stream
 * >NOTE: always assign your pointer to this
 *        function return, because pointer to
 *        stream can be changed
 */
scstr_t *scstr_addn(scstr_t *sc,char *cstr,int len);
/**
 * scstr_len(): get a lenght of fullfilled data
 * (sc: *scstr_t) pointer to a stream
 * return: lenght of actually filled data
 */
size_t scstr_len(scstr_t *sc);

/*clear stream data*/
int scstr_clear(scstr_t *sc);

/*remove stream itself*/
int scstr_remove(scstr_t *sc);

/*get a pointer to stream data*/
char *scstr_get(scstr_t *sc);

#endif /*__SCSTR_H__*/

