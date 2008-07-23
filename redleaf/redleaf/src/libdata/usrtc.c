/*
 * RedLeaf libdata implementation
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
 *
 * Main abstraction implementation (USRTC)
 *
 */ 


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <misc.h>
#include <libdata/usrtc.h>
#include <libdata/macro.h>

/* so, if you want to add your implementation of the data
 * structure please add this here, like the following.
 */

#define MAX_IMPL  4

extern usrtc_functions_t usrtc_list_fu;
extern usrtc_functions_t usrtc_tree_fu;
extern usrtc_functions_t usrtc_redblack_fu;
extern usrtc_functions_t usrtc_splay_fu;
extern usrtc_functions_t usrtc_avl_fu;

static usrtc_functions_t *impl_table[] = {
  &usrtc_list_fu,&usrtc_tree_fu,
  &usrtc_redblack_fu,&usrtc_splay_fu,
  &usrtc_avl_fu };

static usrtc_node_t *default_node_alloc(void *context)
{
  return rl_malloc(sizeof *default_node_alloc(context));
}

static void default_node_free(void *context,usrtc_node_t *node)
{
  rl_free(node);
}

void usrtc_init(usrtc_t *us,int impl,usrtc_count_t maxcount,usrtc_compare_t compare)
{
  if(!us)
    return;
  if(impl>MAX_IMPL)
    return;

  us->nodecount=0;
  us->maxcount=maxcount;
  us->dupes_allowed=0;
  us->compare=compare;
  us->node_alloc=default_node_alloc;
  us->node_free=default_node_free;
  us->context=0;
  us->futable=impl_table[impl];
  us->futable->usrtc_init(us);
}

usrtc_t *usrtc_create(int impl,usrtc_count_t maxcount,usrtc_compare_t compare)
{
  usrtc_t *newrtc=(usrtc_t*)rl_malloc(sizeof *newrtc);
  
  if(newrtc)
    usrtc_init(newrtc,impl,maxcount,compare);
  
  return newrtc;
}

void usrtc_destroy(usrtc_t *us)
{
  /*assert(usrtc_isempty(us));*/
  if(!us)
    return;

  rl_free(us);
}

void usrtc_convert_to(usrtc_t *us,int impl)
{
  if(impl_table[impl]==us->futable)
    return;

  if(us->futable->usrtc_type==usrtc_bst && (impl==USRTC_BST || impl==USRTC_SPLAY)) {
    us->futable=impl_table[impl];
    return;
  }

  us->futable->usrtc_convert_to_list(us);
  us->futable=impl_table[impl];

  if(impl_table[impl]->usrtc_type==usrtc_lst)
    return;

  us->futable->usrtc_convert_from_list(us);
}

usrtc_count_t usrtc_count(usrtc_t *us)
{
  return us->nodecount;
}

int usrtc_isempty(usrtc_t *us)
{
  return us->nodecount == 0;
}

int usrtc_isfull(usrtc_t *us)
{
  return us->nodecount == us->maxcount;
}

int usrtc_alloc_insert(usrtc_t *us,const void *key,void *data)
{
  usrtc_node_t *newnode=us->node_alloc(us->context);
  
  if(newnode!=NULL) {
    newnode->data=data;
    us->futable->usrtc_insert(us,newnode,key);
  }
  
  return newnode!=NULL;
}

void usrtc_delete_free(usrtc_t *us,usrtc_node_t *node)
{
  us->futable->usrtc_delete(us,node);
  us->node_free(us->context,node);
}

void usrtc_set_allocator(usrtc_t *us,usrtc_node_alloc_t alloc,usrtc_node_free_t n_free,void *context)
{
  us->node_alloc=alloc;
  us->node_free=n_free;
  us->context=context;
}

void usrtc_allow_dupes(usrtc_t *us)
{
  /*assert(us->nodecount == 0);*/
  if(!us->nodecount)
    return;

  us->dupes_allowed=1;
}

void usrtc_node_init(usrtc_node_t *node,void *data)
{
  node->data=data;
  node->impl_specific.usrtc_dummy=0;
  node->left=NULL;
  node->right=NULL;
  node->parent=NULL;
}

usrtc_node_t *usrtc_node_create(void *data)
{
  usrtc_node_t *newnode=(usrtc_node_t*)rl_malloc(sizeof *newnode);
  
  if(newnode!=NULL)
    newnode->data=data;
  
  return newnode;
}

void usrtc_node_destroy(usrtc_node_t *node)
{
  rl_free(node);
}

void *usrtc_node_getdata(usrtc_node_t *node)
{
  return node->data;
}

void usrtc_node_setdata(usrtc_node_t *node,void *data)
{
  node->data=data;
}

const void *usrtc_node_getkey(usrtc_node_t *node)
{
  return node->key;
}

void usrtc_insert(usrtc_t *us,usrtc_node_t *node,const void *key)
{
  us->futable->usrtc_insert(us,node,key);
}

void usrtc_delete(usrtc_t *us,usrtc_node_t *node)
{
  us->futable->usrtc_delete(us,node);
}

usrtc_node_t *usrtc_lookup(usrtc_t *us,const void *key)
{
  return us->futable->usrtc_lookup(us,key);
}

usrtc_node_t *usrtc_lower_bound(usrtc_t *us,const void *key)
{
  return us->futable->usrtc_lower_bound(us,key);
}

usrtc_node_t *usrtc_upper_bound(usrtc_t *us,const void *key)
{
  return us->futable->usrtc_upper_bound(us,key);
}

usrtc_node_t *usrtc_first(usrtc_t *us)
{
  return us->futable->usrtc_first(us);
}

usrtc_node_t *usrtc_last(usrtc_t *us)
{
  return us->futable->usrtc_last(us);
}

usrtc_node_t *usrtc_next(usrtc_t *us,usrtc_node_t *node)
{
  return us->futable->usrtc_next(us,node);
}

usrtc_node_t *usrtc_prev(usrtc_t *us,usrtc_node_t *node)
{
  return us->futable->usrtc_prev(us,node);
}


