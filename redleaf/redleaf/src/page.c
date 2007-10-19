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

#include <http.h>
#include <page.h>
#include <misc.h>
#include <libdata/usrtc.h>
#include <libdata/macro.h>

/*local functions prototypes*/

struct page_t *create_page(char *uri,char *head,char *body,char *filename,int op)
{
  struct page_t *page=NULL;

  page=malloc(sizeof(struct page_t));
  if(!page)
    return NULL;
  page->uri=uri;
  page->head=head;
  page->body=body;
  page->filename=filename;
  page->op=op;
  page->last_modify=time(NULL);
  page->last_stat=NULL;
  page->last_access=time(NULL);

  return page;
}
