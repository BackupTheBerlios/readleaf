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
 * Linked list implementation level.
 *
 */ 

#include <stdlib.h>

#include <libdata/macro.h>
#include <libdata/usrtc.h>

#define list_first_priv(L) ((L)->sentinel.next)
#define list_last_priv(L) ((L)->sentinel.prev)
#define list_null_priv(L) (&(L)->sentinel)

/*function prototypes*/
static void list_init(usrtc_t *);
static void list_insert(usrtc_t *,usrtc_node_t *,const void *);
static void list_delete(usrtc_t *,usrtc_node_t *);
static usrtc_node_t *list_lookup(usrtc_t *,const void *);
static usrtc_node_t *list_lower_bound(usrtc_t *,const void *);
static usrtc_node_t *list_upper_bound(usrtc_t *,const void *);
static usrtc_node_t *list_first(usrtc_t *);
static usrtc_node_t *list_last(usrtc_t *);
static usrtc_node_t *list_next(usrtc_t *, usrtc_node_t *);
static usrtc_node_t *list_prev(usrtc_t *, usrtc_node_t *);
static void list_convert_to_list(usrtc_t *);
static void list_convert_from_list(usrtc_t *);

usrtc_functions_t usrtc_list_fu = {
    list_init,
    list_insert,
    list_delete,
    list_lookup,
    list_lower_bound,
    list_upper_bound,
    list_first,
    list_last,
    list_next,
    list_prev,
    list_convert_to_list,
    list_convert_from_list,
    usrtc_lst
};

static void insert_before(usrtc_t *,usrtc_node_t *,usrtc_node_t *);

/*implementation*/
static void list_init(usrtc_t *us)
{
  us->sentinel.next=&us->sentinel;
  us->sentinel.prev=&us->sentinel;
}

static void list_insert(usrtc_t *us,usrtc_node_t *newnode,const void *key)
{
  usrtc_node_t *succ;
  
  newnode->key=key;
  
  for(succ=list_first_priv(us);succ!=list_null_priv(us);succ=succ->next) {
    if(us->compare(succ->key,key)>0)
      break;
  }
  
  insert_before(us,newnode,succ);
}

static void list_delete(usrtc_t *us,usrtc_node_t *node)
{
  usrtc_node_t *pred=node->prev;
  usrtc_node_t *succ=node->next;
  
  pred->next=succ;
  succ->prev=pred;
  /*check node count*/
  us->nodecount--;
}

static usrtc_node_t *list_lookup(usrtc_t *us,const void *key)
{
  usrtc_node_t *node;
  
  for(node=list_first_priv(us);node!=list_null_priv(us);node=node->next) {
    if(us->compare(node->key,key)==0)
      return node;
  }
  
  return NULL;
}

static usrtc_node_t *list_lower_bound(usrtc_t *us,const void *key)
{
  usrtc_node_t *node;
  
  for(node=list_first_priv(us);node!=list_null_priv(us);node=node->next) {
    if(us->compare(node->key,key) >= 0)
      return node;
  }
  
  return NULL;
}

static usrtc_node_t *list_upper_bound(usrtc_t *us,const void *key)
{
  usrtc_node_t *node;
  
  for(node=list_first_priv(us);node!=list_null_priv(us);node=node->prev) {
    if(us->compare(node->key,key) >= 0)
      return node;
  }
  
  return NULL;
}

static usrtc_node_t *list_first(usrtc_t *us)
{
  return (list_first_priv(us) == list_null_priv(us)) ? 0 : list_first_priv(us);
}

static usrtc_node_t *list_last(usrtc_t *us)
{
  return (list_last_priv(us) == list_null_priv(us)) ? 0 : list_last_priv(us);
}

static usrtc_node_t *list_next(usrtc_t *us, usrtc_node_t *node)
{
  return (node->next == list_null_priv(us)) ? 0 : node->next;
}

static usrtc_node_t *list_prev(usrtc_t *us, usrtc_node_t *node)
{
  return (node->prev == list_null_priv(us)) ? 0 : node->prev;
}

static void list_convert_to_list(usrtc_t *us)
{
  /*dummy*/
}

static void list_convert_from_list(usrtc_t *us)
{
  /*dummy*/
}

/*internal function*/
static void insert_before(usrtc_t *us,usrtc_node_t *newnode,usrtc_node_t *succ)
{
  usrtc_node_t *pred=succ->prev;
  
  newnode->prev=pred;
  newnode->next=succ;
  
  pred->next=newnode;
  succ->prev=newnode;
  
  /*TODO: check for maxcount*/
  us->nodecount++;
}

