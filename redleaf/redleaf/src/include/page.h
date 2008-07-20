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

#define CACHE_TIMEOUT  60

/*page_t abstraction*/
struct page_t {
  char *uri; /*url of the page*/
  char *head; /*header of the reply*/
  void *body; /*body of the message*/
  char *filename; /*filename, maybe NULL if content of file is cached*/

  size_t head_len;
  size_t bodysize; /*length of the body*/
  size_t range; /*range offset*/

  time_t last_modify; /*last change of the page*/
  time_t last_stat; /*last modify for file*/
  time_t last_access; /*last access to the page*/

  int op; /* 1 if the file cached with content 
	   * 2 if the file cached without content
	   * 3 if the file is directory
	   * http error - if it's a error
	   */
  int ref;
};

/*initial functions for page_t*/
struct page_t *create_page_t(char *uri,char *head,char *body,char *filename,int op);
void free_page_t(struct page_t *page);
int normalize_page(struct page_t *page);
int denormalize_page(struct page_t *page);
/*cache functions - tree lookup and insert/deletion part*/
int init_page_t_cache(void);
int insert_cache(struct page_t *page);
struct page_t *lookup_cache(char *uri);

#endif /*__PAGE_H__*/
