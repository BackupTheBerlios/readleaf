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
 * Splay tree implementation level
 *
 */ 

#include <libdata/usrtc.h>
#include <libdata/macro.h>
#include <libdata/tree.h>

/*prototypes*/
static void splay_insert(usrtc_t *,usrtc_node_t *,const void *);
static void splay_delete(usrtc_t *,usrtc_node_t *);
static usrtc_node_t *splay_lookup(usrtc_t *,const void *);

usrtc_functions_t usrtc_splay_fu = {
    usrtc_tree_init,
    splay_insert,
    splay_delete,
    splay_lookup,
    usrtc_tree_lower_bound,
    usrtc_tree_upper_bound,
    usrtc_tree_first,
    usrtc_tree_last,
    usrtc_tree_next,
    usrtc_tree_prev,
    usrtc_tree_convert_to_list,
    usrtc_tree_convert_from_list,
    usrtc_bst
};

static void right_zig_zig(usrtc_node_t *,usrtc_node_t *,usrtc_node_t *);
static void left_zig_zig(usrtc_node_t *,usrtc_node_t *,usrtc_node_t *);
static void right_zig_zag(usrtc_node_t *,usrtc_node_t *,usrtc_node_t *);
static void left_zig_zag(usrtc_node_t *,usrtc_node_t *,usrtc_node_t *);
static void splay_node(usrtc_t *,usrtc_node_t *);

/*implementation*/

static void splay_insert(usrtc_t *us,usrtc_node_t *node,const void *key)
{
  usrtc_tree_insert(us,node,key);
  
  while(node!=tree_root_priv(us))
    splay_node(us,node);
}

static void splay_delete(usrtc_t *us,usrtc_node_t *node)
{
  usrtc_node_t *dummy;
  usrtc_tree_delete(us,node,&dummy,&dummy);
}

static usrtc_node_t *splay_lookup(usrtc_t *us,const void *key)
{
  usrtc_node_t *node=usrtc_tree_lookup(us,key);
  
  if(node)
    while(node!=tree_root_priv(us))
      splay_node(us,node);
  
  return node;
}

static void right_zig_zig(usrtc_node_t *child,usrtc_node_t *parent,usrtc_node_t *grandpa)
{
  usrtc_tree_rotate_right(parent,grandpa);
  usrtc_tree_rotate_right(child,parent);
}

static void left_zig_zig(usrtc_node_t *child,usrtc_node_t *parent,usrtc_node_t *grandpa)
{
  usrtc_tree_rotate_left(parent,grandpa);
  usrtc_tree_rotate_left(child,parent);
}

static void right_zig_zag(usrtc_node_t *child,usrtc_node_t *parent,usrtc_node_t *grandpa)
{
  usrtc_tree_rotate_right(child,parent);
  usrtc_tree_rotate_left(child,grandpa);
}

static void left_zig_zag(usrtc_node_t *child,usrtc_node_t *parent,usrtc_node_t *grandpa)
{
  usrtc_tree_rotate_left(child,parent);
  usrtc_tree_rotate_right(child,grandpa);
}

static void splay_node(usrtc_t *us,usrtc_node_t *node)
{
  usrtc_node_t *root=tree_root_priv(us);
  usrtc_node_t *parent=node->parent;
  
  if(parent->left==node) {
    if(parent==root) {
      usrtc_tree_rotate_right(node,parent);
    } else {
      usrtc_node_t *grandpa=parent->parent;
      
      if(grandpa->left==parent)
	right_zig_zig(node,parent,grandpa);
      else {
	if(grandpa->right!=parent)
	  return;

	right_zig_zag(node,parent,grandpa);
      }
    }
  } else {
    if(parent->right!=node)
      return;
    
    if(parent==root) {
      usrtc_tree_rotate_left(node,parent);
    } else {
      usrtc_node_t *grandpa=parent->parent;
      
      if(grandpa->right==parent) {
	left_zig_zig(node,parent,grandpa);
      } else {
	if(grandpa->left!=parent)
	  return;

	left_zig_zag(node,parent,grandpa);
      }
    }
    
  }
}
