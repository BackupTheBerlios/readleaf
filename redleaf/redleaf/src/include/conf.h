/*
 * RedLeaf configuration manager 
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

#ifndef __CONF_H__
#define __CONF_H__

/*configuration*/

struct variable {
  char *var;
  char *value;
};

char *get_general_value(const char *name);
struct variable *get_module_variables(const char *module);
struct variable *get_directory_variables(const char *directory);
struct variable *get_virtualhost_variables(const char *virtualhost);

/*----init-------*/
/*init default general values*/

/*----parser-----*/
/*read syntax and context tree*/
//int read_syn_tree(char *buffer,int size);
void load_configuration(char *buffer,int size);

#endif
