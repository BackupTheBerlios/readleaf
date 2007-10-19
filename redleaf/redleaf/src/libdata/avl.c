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
 * AVL tree implememtation level
 *
 */ 

#include <libdata/usrtc.h>
#include <libdata/macro.h>
#include <libdata/tree.h>

#define balance     usrtc_impldata.usrtc_avl_balance
#define BALANCED    usrtc_balanced
#define LEFTHEAVY   usrtc_leftheavy
#define RIGHTHEAVY  usrtc_rightheavy

/*local functions prototypes*/
static void avl_init(usrtc_t *);
static void avl_insert(usrtc_t *,usrtc_node_t *,const void *);
static void avl_delete(usrtc_t *,usrtc_node_t *);
static void avl_convert_from_list(usrtc_t *);

usrtc_functions_t usrtc_avl_fu = {
    avl_init,
    avl_insert,
    avl_delete,
    usrtc_tree_lookup,
    usrtc_tree_lower_bound,
    usrtc_tree_upper_bound,
    usrtc_tree_first,
    usrtc_tree_last,
    usrtc_tree_next,
    usrtc_tree_prev,
    usrtc_tree_convert_to_list,
    avl_convert_from_list,
    usrtc_bst
};

/*internal use*/
static void rotate_left(usrtc_node_t **);
static void rotate_right(usrtc_node_t **);
static void fix_balance(usrtc_node_t **,usrtc_avl_balance_t );
static int insert(usrtc_t *,usrtc_node_t *,usrtc_node_t **,usrtc_node_t *);
static usrtc_node_t *make_tree(usrtc_node_t **,usrtc_count_t ,int *,usrtc_node_t *);

/*implementation*/
static void avl_init(usrtc_t *us)
{
  usrtc_tree_init(us);
  us->sentinel.balance=BALANCED;
}

static void avl_insert(usrtc_t *us,usrtc_node_t *node,const void *key)
{
  usrtc_node_t *nil=tree_null_priv(us);
  
  node->key=key;
  node->left=nil;
  node->right=nil;
  node->balance=BALANCED;
  
  if(insert(us,node,&nil->left,nil)) {
    nil->balance=LEFTHEAVY;
  }
  
  us->nodecount++;
}

static void avl_delete(usrtc_t *us,usrtc_node_t *node)
{
  usrtc_node_t *nil=tree_null_priv(us);
  usrtc_node_t *swap;
  usrtc_node_t *child;
  usrtc_node_t *parent;

  /*basic deletion*/
  usrtc_tree_delete(us,node,&swap,&child);

  /*implementation specific*/
  swap->balance=node->balance;
  parent=child->parent;

  if(parent==nil) {
    if(child==nil) {
      parent->balance=BALANCED;
    }
  }

  while(parent!=nil) {
    if((parent->left==nil) && (parent->right==nil)) {
      if(child!=nil)
	return;
      if(parent==BALANCED)
	return;
      parent->balance=BALANCED;
    }
    else {
      usrtc_node_t **pparent;
      if(parent==parent->parent->left)
	pparent=&parent->parent->left;
      else
	pparent=&parent->parent->right;

      if(child==parent->left)
	fix_balance(pparent,RIGHTHEAVY);
      else {
	if(child!=parent->right)
	  return;

	fix_balance(pparent,LEFTHEAVY);
      }

      parent=*pparent;
    }

    if(parent->balance==BALANCED) {
      child=parent;
      parent=child->parent;
    }
    else
      break;
  }
}

static void avl_convert_from_list(usrtc_t *us)
{
  usrtc_node_t *nil=tree_null_priv(us);
  usrtc_node_t *root;
  int height;
  
  if(us->nodecount==0) {
    nil->balance=BALANCED;
    return;
  }
  
  root=make_tree(&nil->next,us->nodecount,&height,nil);

  if(nil->next!=nil)
    return;
  
  nil->left=root;
  root->parent=nil;
  nil->balance=LEFTHEAVY;
}

/*---*/
static void rotate_left(usrtc_node_t **top)
{
  usrtc_node_t *parent=*top;
  usrtc_node_t *child=parent->right;
  
  child->parent=parent->parent;
  parent->right=child->left;
  parent->right->parent=parent;
  child->left=parent;
  parent->parent=child;
  *top=child;
}

static void rotate_right(usrtc_node_t **top)
{
  usrtc_node_t *parent=*top;
  usrtc_node_t *child=parent->left;
  
  child->parent=parent->parent;
  parent->left=child->right;
  parent->left->parent=parent;
  child->right=parent;
  parent->parent=child;
  *top=child;
}

static void fix_balance(usrtc_node_t **pnode,usrtc_avl_balance_t bal)
{
  usrtc_node_t *node=*pnode;
  usrtc_node_t *child;
  usrtc_node_t *grandchild;

  if(node->balance==BALANCED)
    node->balance=bal;
  else if(node->balance!=bal)
    node->balance=BALANCED;
  else {
    if(node->balance!=bal)
      return;

    if(bal==LEFTHEAVY) {
      child=node->left;
      if(child->balance==LEFTHEAVY) {
	node->balance=BALANCED;
	child->balance=BALANCED;
	rotate_right(pnode);
      }
      else if(child->balance==BALANCED) {
	node->balance=LEFTHEAVY;
	child->balance=RIGHTHEAVY;
	rotate_right(pnode);
      }
      else {
	grandchild=child->right;
	if(grandchild->balance==LEFTHEAVY) {
	  node->balance=RIGHTHEAVY;
	  child->balance=BALANCED;
	}
	else if(grandchild->balance==RIGHTHEAVY) {
	  node->balance=BALANCED;
	  child->balance=LEFTHEAVY;
	}
	else {
	  node->balance=BALANCED;
	  child->balance=BALANCED;
	}
	grandchild->balance=BALANCED;
	rotate_left(&node->left);
	rotate_right(pnode);
      }
    }
    else {
      child=node->right;
      if(child->balance==RIGHTHEAVY) {
	node->balance=BALANCED;
	child->balance=BALANCED;
	rotate_left(pnode);
      }
      else if(child->balance==BALANCED) {
	node->balance=RIGHTHEAVY;
	child->balance=LEFTHEAVY;
	rotate_left(pnode);
      }
      else {
	grandchild=child->left;
	if(grandchild->balance==RIGHTHEAVY) {
	  node->balance=LEFTHEAVY;
	  child->balance=BALANCED;
	}
	else if(grandchild->balance==LEFTHEAVY) {
	  node->balance=BALANCED;
	  child->balance=RIGHTHEAVY;
	}
	else {
	  node->balance=BALANCED;
	  child->balance=BALANCED;
	}
	grandchild->balance=BALANCED;
	rotate_right(&node->right);
	rotate_left(pnode);
      }
    }
  }
}

static int insert(usrtc_t *us,usrtc_node_t *what,usrtc_node_t **where,usrtc_node_t *parent)
{
  usrtc_node_t *here=*where;
  int result;

  if(here==tree_null_priv(us)) {
    *where=what;
    what->parent=parent;
    return 1;
  }
  else {
    result=us->compare(what->key,here->key);	

    if(result < 0) {
      if(insert(us,what,&here->left,here)) {
	fix_balance(where,LEFTHEAVY);
	return ((*where)->balance!=BALANCED);
      }
    }
    else {
      if(insert(us,what,&here->right,here)) {
	fix_balance(where,RIGHTHEAVY);
	return ((*where)->balance!=BALANCED);
      }
    }
  }
  return 0;
}

static usrtc_node_t *make_tree(usrtc_node_t **pnode,usrtc_count_t count,
			       int *pheight,usrtc_node_t *nil)
{
  usrtc_count_t leftcount;
  int leftheight, rightheight;
  usrtc_node_t *root;
  usrtc_node_t *leftroot;

  if(count==0) {
    *pheight=0;
    return nil;
  }

  leftcount=(count-1)/2;
  leftroot=make_tree(pnode,leftcount,&leftheight,nil);
  count-=leftcount;

  root=*pnode;
  *pnode=root->next;
  --count;

  root->left=leftroot;
  leftroot->parent=root;
  root->right=make_tree(pnode,count,&rightheight,nil);
  root->right->parent=root;

  if(leftheight>rightheight)
    return (void *)0;

  *pheight=rightheight+1;
  root->balance=(leftheight==rightheight) ? BALANCED : RIGHTHEAVY;

  return root;
}

