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

#ifndef __TREE_H__
#define __TREE_H__

#include <libdata/usrtc.h>

#define tree_root_priv(T) ((T)->sentinel.left)
#define tree_null_priv(L) (&(L)->sentinel)
#define TREE_DEPTH_MAX 64

/*tree functions*/
void usrtc_tree_init(usrtc_t *us);
void usrtc_tree_insert(usrtc_t *us,usrtc_node_t *node,const void *key);
void usrtc_tree_delete(usrtc_t *us,usrtc_node_t *node,usrtc_node_t **pswap,usrtc_node_t **pchild);
usrtc_node_t *usrtc_tree_lookup(usrtc_t *us,const void *key);
usrtc_node_t *usrtc_tree_lower_bound(usrtc_t *us,const void *key);
usrtc_node_t *usrtc_tree_upper_bound(usrtc_t *us,const void *key);
usrtc_node_t *usrtc_tree_first(usrtc_t *us);
usrtc_node_t *usrtc_tree_last(usrtc_t *us);
usrtc_node_t *usrtc_tree_next(usrtc_t *us, usrtc_node_t *curr);
usrtc_node_t *usrtc_tree_prev(usrtc_t *us, usrtc_node_t *curr);
void usrtc_tree_convert_to_list(usrtc_t *us);
void usrtc_tree_convert_from_list(usrtc_t *us);
void usrtc_tree_rotate_left(usrtc_node_t *child,usrtc_node_t *parent);
void usrtc_tree_rotate_right(usrtc_node_t *child,usrtc_node_t *parent);


#endif /*__TREE_H__*/
