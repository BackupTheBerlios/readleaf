/*
 * RedLeaf page_t operations, cache and processing
 *
 * Copyright (C) 2006, 2007 RedLeaf devteam org.
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

#ifndef __PAGE_H__
#define __PAGE_H__

/*page_t abstraction*/
struct page_t {
  char *uri; /*url of the page*/
  char *head; /*header of the reply*/
  char *body; /*body of the message*/
  char *filename; /*filename, maybe NULL if content of file is cached*/

  time_t last_modify; /*last change of the page*/
  time_t last_stat; /*last stat() for file*/
  time_t last_access; /*last access to the page*/

  int op; /*0 if the cached request, 
	   * 1 if the cached request with file
	   * error if it's the system message
	   */
};

#endif /*__PAGE_H__*/
