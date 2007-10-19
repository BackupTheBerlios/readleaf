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
 * RedBlack tree implementation level
 *
 */ 

#include <stdlib.h>

#include <libdata/usrtc.h>
#include <libdata/macro.h>
#include <libdata/tree.h>

#define color usrtc_impldata.usrtc_rb_color

/*function prototypes*/
static void redblack_init(usrtc_t *);
static void redblack_insert(usrtc_t *,usrtc_node_t *,const void *);
static void redblack_delete(usrtc_t *, usrtc_node_t *);
static void redblack_convert_from_list(usrtc_t *);

usrtc_functions_t usrtc_redblack_fu = {
    redblack_init,
    redblack_insert,
    redblack_delete,
    usrtc_tree_lookup,
    usrtc_tree_lower_bound,
    usrtc_tree_upper_bound,
    usrtc_tree_first,
    usrtc_tree_last,
    usrtc_tree_next,
    usrtc_tree_prev,
    usrtc_tree_convert_to_list,
    redblack_convert_from_list,
    usrtc_bst
};

/*implementation*/

static void redblack_init(usrtc_t *us)
{
  usrtc_tree_init(us);
  us->sentinel.color=usrtc_black;
}

static void redblack_insert(usrtc_t *us,usrtc_node_t *node,const void *key)
{
  usrtc_node_t *parent;
  
  /*simple bt insert */
  usrtc_tree_insert(us,node,key);
  
  /*implementation specific insert*/
  node->color=usrtc_red;
  parent=node->parent;
  
  while(parent->color==usrtc_red) {
    usrtc_node_t *grandpa=parent->parent;
    
    if(parent==grandpa->left) {
      usrtc_node_t *uncle=grandpa->right;
      if(uncle->color==usrtc_red) { /*red parent->red uncle*/
	parent->color=usrtc_black;
	uncle->color=usrtc_black;
	grandpa->color=usrtc_red;
	node=grandpa;
	parent=grandpa->parent;
      } else { /*red parent->black uncle */
	if(node==parent->right) {
	  usrtc_tree_rotate_left(node,parent);
	  parent=node;
	  if(grandpa!=parent->parent)
	    return;
	}
	parent->color=usrtc_black;
	grandpa->color=usrtc_red;
	usrtc_tree_rotate_right(parent,grandpa);
	break;
      }
    } else { /*ooh,parent == parent->parent->right*/
      usrtc_node_t *uncle=grandpa->left;
      if(uncle->color==usrtc_red) {
	parent->color=usrtc_black;
	uncle->color=usrtc_black;
	grandpa->color=usrtc_red;
	node=grandpa;
	parent=grandpa->parent;
      } else {
	if(node==parent->left) {
	  usrtc_tree_rotate_right(node,parent);
	  parent=node;

	  if(grandpa!=parent->parent)
	    return;
	}
	parent->color=usrtc_black;
	grandpa->color=usrtc_red;
	usrtc_tree_rotate_left(parent,grandpa);
	break;
      }
    }
  }
  
  tree_root_priv(us)->color=usrtc_black;
}

static void redblack_delete(usrtc_t *us,usrtc_node_t *node)
{
  usrtc_node_t *swap;
  usrtc_node_t *child;
  usrtc_rb_color_t savecolor;

  /*basic bt delete*/
  usrtc_tree_delete(us,node,&swap,&child);
    
  /*implementation specific deletion*/
  savecolor=node->color;
  node->color=swap->color;
  swap->color=savecolor;

  if(node->color==usrtc_black) { /*black*/
    usrtc_node_t *parent;
    usrtc_node_t *sister;

    tree_root_priv(us)->color=usrtc_red;

    while(child->color==usrtc_black) {
      parent=child->parent;
      if(child==parent->left) {
	sister=parent->right;
	if(sister==tree_null_priv(us))
	  return;

	if(sister->color==usrtc_red) {
	  sister->color=usrtc_black;
	  parent->color=usrtc_red;
	  usrtc_tree_rotate_left(sister,parent);
	  sister=parent->right;

	  if(sister==tree_null_priv(us))
	    return;
	}
	if(sister->left->color==usrtc_black && sister->right->color==usrtc_black) {
	  sister->color=usrtc_red;
	  child=parent;
	} else {
	  if(sister->right->color==usrtc_black) {
	    if(sister->left->color!=usrtc_red)
	      return;

	    sister->left->color=usrtc_black;
	    sister->color=usrtc_red;
	    usrtc_tree_rotate_right(sister->left,sister);
	    sister=parent->right;

	    if(sister==tree_null_priv(us))
	      return;
	  }
	  sister->color=parent->color;
	  sister->right->color=usrtc_black;
	  parent->color=usrtc_black;
	  usrtc_tree_rotate_left(sister,parent);
	  break;
	}
      } else {	/*!child == child->parent->right*/
	if(child!=parent->right)
	  return;

	sister=parent->left;

	if(sister==tree_null_priv(us))
	  return;

	if(sister->color==usrtc_red) {
	  sister->color=usrtc_black;
	  parent->color=usrtc_red;
	  usrtc_tree_rotate_right(sister,parent);
	  sister = parent->left;

	  if(sister==tree_null_priv(us))
	    return;
	}
	if(sister->right->color==usrtc_black && sister->left->color==usrtc_black) {
	  sister->color=usrtc_red;
	  child=parent;
	} else {
	  if(sister->left->color == usrtc_black) {
	    if(sister->right->color!=usrtc_red)
	      return;

	    sister->right->color=usrtc_black;
	    sister->color=usrtc_red;
	    usrtc_tree_rotate_left(sister->right, sister);
	    sister=parent->left;

	    if(sister==tree_null_priv(us))
	      return;
	  }
	  sister->color=parent->color;
	  sister->left->color=usrtc_black;
	  parent->color=usrtc_black;
	  usrtc_tree_rotate_right(sister,parent);
	  break;
	}
      }
    }

    child->color=usrtc_black;
    tree_root_priv(us)->color=usrtc_black;
  }

}

static void redblack_convert_from_list(usrtc_t *us)
{
  usrtc_node_t *tree[TREE_DEPTH_MAX] = { NULL };
  usrtc_node_t *curr;
  usrtc_node_t *nil=tree_null_priv(us);
  usrtc_node_t *next;
  usrtc_node_t *complete=NULL;
  usrtc_count_t fullcount=USRTC_COUNT_T_MAX;
  usrtc_count_t nodecount=us->nodecount;
  usrtc_count_t botrowcount;
  unsigned int baselevel=0;
  unsigned int level=0;
  unsigned int i;

  if(usrtc_red!=0 && usrtc_black!=1)
    return;

  while(fullcount>=nodecount && fullcount) /*calc*/
    fullcount >>= 1;

  botrowcount=nodecount-fullcount;

  for(curr=nil->next;curr!=nil;curr=next) {
    next=curr->next;

    if(complete==NULL && botrowcount--==0) {
      baselevel=level=1;
      complete=tree[0];

      if(complete!=NULL) {
	tree[0]=NULL;
	complete->right=nil;
	while(tree[level]!=0) {
	  tree[level]->right=complete;
	  complete->parent=tree[level];
	  complete=tree[level];
	  tree[level++]=NULL;
	}
      }
    }

    if(complete==NULL) {
      curr->left=nil;
      curr->right=nil;
      curr->color=level%2;
      complete=curr;

      if(level!=baselevel)
	return;

      while(tree[level]!=NULL) {
	tree[level]->right=complete;
	complete->parent=tree[level];
	complete=tree[level];
	tree[level++]=NULL;
      }
    } else {
      curr->left=complete;
      curr->color=(level+1)%2;
      complete->parent=curr;
      tree[level]=curr;
      complete=NULL;
      level=baselevel;
    }
  }

  if(complete==NULL)
    complete=nil;

  for (i=0;i<TREE_DEPTH_MAX;i++) {
    if (tree[i]!=NULL) {
      tree[i]->right=complete;
      complete->parent=tree[i];
      complete=tree[i];
    }
  }

  nil->right=nil;
  nil->left=complete;
  nil->color=usrtc_black;
  complete->parent=nil;
  complete->color=usrtc_black;

}

