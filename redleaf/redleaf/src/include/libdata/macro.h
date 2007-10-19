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

#ifndef __MACRO_H__
#define __MACRO_H__

/*this macros just for better source code look and feel*/

#define left usrtc_left
#define right usrtc_right
#define parent usrtc_parent
#define next usrtc_right
#define prev usrtc_left
#define data usrtc_data
#define key usrtc_node_key
#define rb_color usrtc_rb_color
#define impl_specific usrtc_impldata

#define futable usrtc_futable
#define nodecount usrtc_nodecount
#define maxcount usrtc_maxcount
#define dupes_allowed usrtc_dupes_allowed
#define sentinel usrtc_sentinel
#define compare usrtc_compare
#define node_alloc usrtc_node_alloc
#define node_free usrtc_node_free
#define context usrtc_context

#endif /*__MACRO_H__*/
