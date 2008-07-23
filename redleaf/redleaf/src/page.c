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

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>

#include <http.h>
#include <page.h>
#include <misc.h>
#include <libdata/usrtc.h>
#include <libdata/macro.h>

/*locally used variables*/
usrtc_t *cache_uri=NULL;
int cached_pages=0;

/*global functions prototypes*/
struct page_t *create_page_t(char *uri,char *head,char *body,char *filename,int op);
void free_page_t(struct page_t *page);

/*local functions prototypes*/
static int compare_function(const void *a, const void *b);

int init_page_t_cache(void)
{
  cache_uri=usrtc_create(USRTC_SPLAY,65535,compare_function);
  if(!cache_uri)
    return -1;

  return 0;
}

int insert_cache(struct page_t *page)
{
  usrtc_node_t *nnode=NULL;

  if(!cache_uri)
    return -1;

  /*change time values*/
  page->last_modify=page->last_access=time(NULL);
  cached_pages++;
  /*TODO: implement a cache size*/
  if(cached_pages>65535) { /*destroy the last cached page,in the case of it has a far access time*/
    nnode=usrtc_last(cache_uri);
    free_page_t((struct page_t *)usrtc_node_getdata(nnode));
    usrtc_node_destroy(nnode);
    cached_pages--;
  }

  nnode=usrtc_node_create((void *)page); /*create node and set data*/
  usrtc_node_setdata(nnode,(void *)page);
  usrtc_insert(cache_uri,nnode,(void *)page->uri); /*insert new node*/

  return 0;
}

struct page_t *lookup_cache(char *uri)
{
  usrtc_node_t *nnode=NULL;
  struct page_t *page=NULL;

  nnode=usrtc_lookup(cache_uri,(void *)uri);
  if(!nnode)
    return NULL;
  else { 
    page=(struct page_t*)usrtc_node_getdata(nnode);
    page->last_access=time(NULL);
    return page;
  }

  return NULL;
}

/*initial operations for page_t*/
struct page_t *create_page_t(char *uri,char *head,char *body,char *filename,int op)
{
  struct page_t *page=NULL;

  page=rl_malloc(sizeof(struct page_t));
  page->uri=uri;
  page->head=head;
  page->body=body;
  page->filename=filename;
  page->op=op;
  page->last_modify=time(NULL);
  page->last_stat=time(NULL);
  page->last_access=time(NULL);
  page->bodysize=0;
  page->range=0;
  page->ref=0;

  return page;
}

void free_page_t(struct page_t *page)
{
  if(!page)
    return;
  if(page->uri)
    rl_free(page->uri);
  if(page->head)
    rl_free(page->head);
  if(page->body)
    rl_free(page->body);
  if(page->filename)
    rl_free(page->filename);
  rl_free(page);

  return;
}

int normalize_page(struct page_t *page)
{
  int total_len=page->head_len+page->bodysize;
  char *data=NULL,*data_r;

  if(page->op==2) /*normal, in the case of big file*/
    return 0;

  data=rl_malloc(total_len);
  data_r=data;
  memcpy(data,page->head,page->head_len);
  data_r+=page->head_len;
  memcpy(data_r,page->body,page->bodysize);
  rl_free(page->head);
  rl_free(page->body);
  page->head=data;
  page->body=data_r;

  return 0;
}

int denormalize_page(struct page_t *page)
{
  char *head,*body;

  if(page->op==2) /*normal, in the case of big file*/
    return 0;
  head=rl_malloc(page->head_len);
  head=memcpy(head,page->head,page->head_len);
  body=rl_malloc(page->bodysize);
  body=memcpy(body,page->body,page->bodysize);

  rl_free(page->head);

  page->head=head;
  page->body=body;

  return 0;
}

/*local functions*/
static int compare_function(const void *a, const void *b)
{
  return strcmp((const char *)a,(const char *)b);
}
