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
 */ 

#ifndef __USRTC_H__
#define __USRTC_H__

#define USRTC_COUNT_T_MAX  ((1 << (sizeof(unsigned long)*(sizeof(char) << 1)))-1)

/*count and some misc typedefs*/
typedef unsigned long usrtc_count_t;
typedef unsigned int usrtc_impl_id_t;

/* implementations ids,
 * if you want to add some
 * add the id here, don't forget to do it.
 */
#define USRTC_LIST      0
#define USRTC_BST       1
#define USRTC_REDBLACK  2
#define USRTC_SPLAY     3
#define USRTC_AVL       4

typedef enum {
  usrtc_lst,
  usrtc_bst,
  usrtc_other
} usrtc_impltype_t;

typedef enum {
  usrtc_red,
  usrtc_black
} usrtc_rb_color_t;
typedef enum {
  usrtc_balanced,
  usrtc_leftheavy,
  usrtc_rightheavy
} usrtc_avl_balance_t;

/*used for the specific implementation features*/
typedef union {
  int usrtc_dummy;
  usrtc_rb_color_t usrtc_rb_color;
  usrtc_avl_balance_t usrtc_avl_balance;
} usrtc_impldata_t;

/*universal node*/
typedef struct __usrtc_node_t {
  struct __usrtc_node_t *usrtc_left;
  struct __usrtc_node_t *usrtc_right;
  struct __usrtc_node_t *usrtc_parent;
  void *usrtc_data;
  const void *usrtc_node_key;
  usrtc_impldata_t usrtc_impldata;
} usrtc_node_t;

typedef int (*usrtc_compare_t)(const void*, const void*);
typedef usrtc_node_t *(*usrtc_node_alloc_t)(void *);
typedef void (*usrtc_node_free_t)(void *, usrtc_node_t *);

typedef struct __usrtc_t {
    struct __usrtc_functions_t *usrtc_futable;
    usrtc_count_t usrtc_nodecount;
    usrtc_count_t usrtc_maxcount;
    int usrtc_dupes_allowed;
    usrtc_node_t usrtc_sentinel;
    usrtc_compare_t usrtc_compare;
    usrtc_node_alloc_t usrtc_node_alloc;
    usrtc_node_free_t usrtc_node_free;
    void *usrtc_context;
} usrtc_t;
                                                                                                           
typedef struct __usrtc_functions_t {
    void (*usrtc_init)(usrtc_t *);
    void (*usrtc_insert)(usrtc_t *, usrtc_node_t *, const void *);
    void (*usrtc_delete)(usrtc_t *, usrtc_node_t *);
    usrtc_node_t *(*usrtc_lookup)(usrtc_t *, const void *);
    usrtc_node_t *(*usrtc_lower_bound)(usrtc_t *, const void *);
    usrtc_node_t *(*usrtc_upper_bound)(usrtc_t *, const void *);
    usrtc_node_t *(*usrtc_first)(usrtc_t *);
    usrtc_node_t *(*usrtc_last)(usrtc_t *);
    usrtc_node_t *(*usrtc_next)(usrtc_t *, usrtc_node_t *);
    usrtc_node_t *(*usrtc_prev)(usrtc_t *, usrtc_node_t *);
    void (*usrtc_convert_to_list)(usrtc_t *);
    void (*usrtc_convert_from_list)(usrtc_t *);
    usrtc_impltype_t usrtc_type;
} usrtc_functions_t;

/*basic rtc functions*/
void usrtc_init(usrtc_t *us,int impl,usrtc_count_t maxcount,usrtc_compare_t compare);
usrtc_t *usrtc_create(int impl,usrtc_count_t maxcount,usrtc_compare_t compare);
void usrtc_destroy(usrtc_t *us);
void usrtc_convert_to(usrtc_t *us,int impl);
usrtc_count_t usrtc_count(usrtc_t *us);
int usrtc_isempty(usrtc_t *us);
int usrtc_isfull(usrtc_t *us);
int usrtc_alloc_insert(usrtc_t *us,const void *key,void *data);
void usrtc_delete_free(usrtc_t *us,usrtc_node_t *node);
void usrtc_set_allocator(usrtc_t *us,usrtc_node_alloc_t alloc,usrtc_node_free_t n_free,void *context);
void usrtc_allow_dupes(usrtc_t *ud);

/*basic node functions*/
void usrtc_node_init(usrtc_node_t *node,void *data);
usrtc_node_t *usrtc_node_create(void *data);
void usrtc_node_destroy(usrtc_node_t *node);
void *usrtc_node_getdata(usrtc_node_t *node);
void usrtc_node_setdata(usrtc_node_t *node,void *data);
const void *usrtc_node_getkey(usrtc_node_t *node);

/*rtc wrappers for the specific data structure functions*/
void usrtc_insert(usrtc_t *us,usrtc_node_t *node,const void *key);
void usrtc_delete(usrtc_t *us,usrtc_node_t *node);
usrtc_node_t *usrtc_lookup(usrtc_t *us,const void *key);
usrtc_node_t *usrtc_lower_bound(usrtc_t *us,const void *key);
usrtc_node_t *usrtc_upper_bound(usrtc_t *us,const void *key);
usrtc_node_t *usrtc_first(usrtc_t *us);
usrtc_node_t *usrtc_last(usrtc_t *us);
usrtc_node_t *usrtc_next(usrtc_t *us,usrtc_node_t *node);
usrtc_node_t *usrtc_prev(usrtc_t *us,usrtc_node_t *node);

#endif /*__USRTC_H__*/
