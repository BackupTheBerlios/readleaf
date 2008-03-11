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
 * General abstraction for tree (the upper level on the USRTC)
 *
 */ 

#include <stdlib.h>

#include <libdata/usrtc.h>
#include <libdata/macro.h>
#include <libdata/tree.h>

/*local functions prototypes*/
static void tree_delete(usrtc_t *, usrtc_node_t *);

usrtc_functions_t usrtc_tree_fu = {
    usrtc_tree_init,
    usrtc_tree_insert,
    tree_delete,
    usrtc_tree_lookup,
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

/*implementation*/

void usrtc_tree_init(usrtc_t *us)
{
  us->sentinel.left=&us->sentinel;
  us->sentinel.right=&us->sentinel;
  us->sentinel.parent=&us->sentinel;

  us->sentinel.impl_specific.usrtc_dummy=0;
  us->sentinel.data=0;
  us->sentinel.key=0;
}

void usrtc_tree_insert(usrtc_t *us,usrtc_node_t *node,const void *key)
{
  usrtc_node_t *where=tree_root_priv(us);
  usrtc_node_t *nil=tree_null_priv(us);
  usrtc_node_t *parent=nil;
  int res=-1;
  
  node->key=key;
  
  while(where!=nil) {
    parent=where;
    res=us->compare(key,where->key);

    if(us->dupes_allowed && !res) /*trying to put duplicate to the disabled dupe tree*/
      return;

    if(res<0)
      where=where->left;
    else
      where=where->right;
  }
  
  /*assert(where==nil);*/
  
  if(res<0)
    parent->left=node;
  else
    parent->right=node;

  node->parent=parent;
  node->left=nil;
  node->right=nil;
  
  us->nodecount++;
}

void usrtc_tree_delete(usrtc_t *us,usrtc_node_t *node,usrtc_node_t **pswap,usrtc_node_t **pchild)
{
  usrtc_node_t *nil=tree_null_priv(us); 
  usrtc_node_t *child;
  usrtc_node_t *delparent=node->parent;
  usrtc_node_t *next=node;
  usrtc_node_t *nextparent;
  
  if(node->left!=nil && node->right!=nil) {
    next=usrtc_tree_next(us,node);
    nextparent=next->parent;
    
    /*if(next!=nil && next->parent!=nil && next->parent==nil)
      return;*/

    child=next->right;
    child->parent=nextparent;
    
    if(nextparent->left==next)
      nextparent->left=child;
    else {
      //if(nextparent->right!=next)
      //return;
      nextparent->right=child;
    }
    
    next->parent=delparent;
    next->left=node->left;
    next->right=node->right;
    next->left->parent=next;
    next->right->parent=next;
    
    if(delparent->left==node) {
      delparent->left=next;
    } else {
      //if(delparent->right!=node)
      //	return;
      delparent->right = next;
    }
    
  } else {
    /*if(node==nil)
      return;
    if(node->left!=nil && node->right!=nil)
    return;*/
    
    child=(node->left!=nil) ? node->left : node->right;
    
    child->parent=delparent=node->parent;	    
    
    if(node==delparent->left) {
      delparent->left=child;    
    } else {
      /*if(node!=delparent->right)
	return;*/
      delparent->right=child;
    }
  }
  
  node->parent=0;
  node->right=0;
  node->left=0;
  
  us->nodecount--;
  
  *pswap = next;
  *pchild = child;
}

usrtc_node_t *usrtc_tree_lookup(usrtc_t *us,const void *key)
{
  usrtc_node_t *root=tree_root_priv(us);
  usrtc_node_t *nil=tree_null_priv(us);
  usrtc_node_t *saved;
  int res;
  
  while (root!=nil) {
    res=us->compare(key,root->key);
    if(res<0)
      root=root->left;
    else if(res>0)
      root=root->right;
    else {
      if(!us->dupes_allowed) 
	return root; /*no duplicates*/
      else { /*duplicate, then find left occurence*/
	do {
	  saved=root;
	  root=root->left;
	  while(root!=nil && us->compare(key,root->key))
	    root=root->right;
	} while(root!=nil);
	return saved;
      }
    }
  }
  
  return NULL;
}

usrtc_node_t *usrtc_tree_lower_bound(usrtc_t *us,const void *key)
{
  usrtc_node_t *root=tree_root_priv(us);
  usrtc_node_t *nil=tree_null_priv(us);
  usrtc_node_t *tentative=NULL;
  int res;  

  while(root!=nil) {
    res=us->compare(key,root->key);
    
    if(res>0) {
      root=root->right;
    } else if(res<0) {
      tentative=root;
      root=root->left;
    } else {
      if (!us->dupes_allowed)
	return root;
      else {
	tentative=root;
	root=root->left;
      }
    }
  }
  
  return tentative;
}

usrtc_node_t *usrtc_tree_upper_bound(usrtc_t *us,const void *key)
{
  usrtc_node_t *root=tree_root_priv(us);
  usrtc_node_t *nil=tree_null_priv(us);
  usrtc_node_t *tentative=NULL;
  int res;  

  while(root!=nil) {
    res=us->compare(key,root->key);
    
    if(res>0) {
      root=root->left;
    } else if(res<0) {
      tentative=root;
      root=root->right;
    } else {
      if (!us->dupes_allowed)
	return root;
      else {
	tentative=root;
	root=root->right;
      }
    }
  }
  
  return tentative;
}

usrtc_node_t *usrtc_tree_first(usrtc_t *us)
{
  usrtc_node_t *nil=tree_null_priv(us);
  usrtc_node_t *root=tree_root_priv(us);
  usrtc_node_t *left;
  
  if(root!=nil)
    while((left=root->left)!=nil)
      root=left;
  
  return (root==nil) ? NULL : root;
}

usrtc_node_t *usrtc_tree_last(usrtc_t *us)
{
  usrtc_node_t *nil=tree_null_priv(us);
  usrtc_node_t *root=tree_root_priv(us);
  usrtc_node_t *right;
  
  if(root!=nil)
    while((right=root->right)!=nil)
      root=right;
  
  return (root==nil) ? NULL : root;
}

usrtc_node_t *usrtc_tree_next(usrtc_t *us, usrtc_node_t *curr)
{
  usrtc_node_t *nil=tree_null_priv(us);
  usrtc_node_t *parent;
  usrtc_node_t *left;
  
  if(curr->right!=nil) {
    curr=curr->right;
    while((left=curr->left)!=nil)
      curr=left;
    return curr;
  }
  
  parent=curr->parent;
  
  while(parent!=nil && curr==parent->right) {
    curr=parent;
    parent=curr->parent;
  }
  
  return (parent==nil) ? NULL : parent;
}

usrtc_node_t *usrtc_tree_prev(usrtc_t *us, usrtc_node_t *curr)
{
  usrtc_node_t *nil=tree_null_priv(us);
  usrtc_node_t *parent;
  usrtc_node_t *right;
  
  if(curr->left!=nil) {
    curr=curr->left;
    while((right=curr->right)!=nil)
      curr=right;
    return curr;
  }
  
  parent=curr->parent;
  
  while(parent!=nil && curr==parent->left) {
    curr=parent;
    parent=curr->parent;
  }
  
  return (parent==nil) ? NULL : parent;
}

/*uff, convertation between trees and lists*/

void usrtc_tree_convert_to_list(usrtc_t *us)
{
  usrtc_node_t *node;
  usrtc_node_t tempsentinel;
  usrtc_node_t *nil=&tempsentinel;
  usrtc_node_t *tail,*next;
  usrtc_node_t *treenil=tree_null_priv(us);
  
  if(us->nodecount==0) /*no nodes*/
    return;
  
  tempsentinel.next=nil;
  tempsentinel.prev=nil;

  /*two passes*/
  
  for(tail=nil,node=usrtc_tree_first(us);node!=0;tail=node,node=next) {
    next=usrtc_tree_next(us,node);
    node->prev=tail;
  }
  
  nil->prev=tail;
  
  for(tail=nil,node=nil->prev;node!=nil;tail=node,node=node->prev)
    node->next=tail;
  
  nil->next=tail;
  
  us->sentinel.next=tempsentinel.next;
  us->sentinel.prev=tempsentinel.prev;
  us->sentinel.next->prev=treenil;
  us->sentinel.prev->next=treenil;
}

void usrtc_tree_convert_from_list(usrtc_t *us)
{
  usrtc_node_t *tree[TREE_DEPTH_MAX]={ 0 };
  usrtc_node_t *curr;
  usrtc_node_t *nil=tree_null_priv(us);
  usrtc_node_t *next;
  usrtc_node_t *complete=NULL;
  usrtc_count_t fullcount=(usrtc_count_t)USRTC_COUNT_T_MAX;
  usrtc_count_t nodecount=us->nodecount;
  usrtc_count_t botrowcount;
  int baselevel=0;
  int level=0;
  int i=0;

  while (fullcount>=nodecount && fullcount) /*calc */
    fullcount>>=1;
  
  botrowcount=nodecount-fullcount;
  
  for(curr=nil->next;curr!=nil;curr=next) {
    next=curr->next;
    
    if(complete==NULL && botrowcount-- ==0) {
      baselevel=level=1;
      complete=tree[0];
      
      if(complete!=NULL) {
	tree[0]=0;
	complete->right=nil;
	while(tree[level]!=NULL) {
	  tree[level]->right=complete;
	  complete->parent=tree[level];
	  complete=tree[level];
	  tree[level++]=0;
	}
      }
    }
    
    if(complete==NULL) {
      curr->left=nil;
      curr->right=nil;
      complete=curr;
      
      if(level!=baselevel)
	return;
      while(tree[level]!=NULL) {
	tree[level]->right=complete;
	complete->parent=tree[level];
	complete=tree[level];
	tree[level++]=0;
      }
    } else {
      curr->left=complete;
      complete->parent=curr;
      tree[level]=curr;
      complete=NULL;
      level=baselevel;
    }
  }
  
  if(complete==NULL)
    complete=nil;
  
  for(i=0;i<TREE_DEPTH_MAX;i++) {
    if(tree[i]!=NULL) {
      tree[i]->right=complete;
      complete->parent=tree[i];
      complete=tree[i];
    }
  }
  
  nil->right=nil;
  nil->left=complete;
  complete->parent=nil;

}

void usrtc_tree_rotate_left(usrtc_node_t *child,usrtc_node_t *parent)
{
  usrtc_node_t *leftgrandchild;
  usrtc_node_t *grandpa;

  if(parent->right!=child)
    return;

  child=parent->right;
  parent->right=leftgrandchild=child->left;
  leftgrandchild->parent=parent;

  child->parent=grandpa=parent->parent;
  
  if(parent==grandpa->left) {
    grandpa->left=child;
  } else {
    if(parent!=grandpa->right)
      return;
    grandpa->right=child;
  }
  
  child->left=parent;
  parent->parent=child;
}

void usrtc_tree_rotate_right(usrtc_node_t *child,usrtc_node_t *parent)
{
  usrtc_node_t *rightgrandchild;
  usrtc_node_t *grandpa;

  if(parent->left!=child)
    return;

  parent->left=rightgrandchild=child->right;
  rightgrandchild->parent=parent;

  child->parent=grandpa=parent->parent;
  
  if(parent==grandpa->right) {
    grandpa->right=child;
  } else {
    if(parent!=grandpa->left)
      return;
    grandpa->left=child;
  }
  
  child->right=parent;
  parent->parent=child;
}

/*local functions*/

static void tree_delete(usrtc_t *us, usrtc_node_t *node)
{
  usrtc_node_t *dummy;

  usrtc_tree_delete(us, node, &dummy, &dummy);
}

