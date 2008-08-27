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

#include <sys/types.h>

#include "../../config.h"

#include <libdata/usrtc.h>

/*configuration structures*/

#define HOST_ALLOW_LISTING  (1 << 0)
#ifdef MODULAS
#define HOST_ALLOW_MODULAS  (1 << 1)
#define HOST_ALLOW_EXEC     (1 << 2)
#endif

/**
 * host_config_t:
 * used for locate information about host
 */
typedef struct __host_config_t { 
  char *host_url; /*url of host*/
  char *host_root_dir; /*root dir*/
  char **index;   /*list of indexing files*/
  int flags;
} host_config_t;

/**
 * runtime_conf_t:
 * used for operate with configuration data on runtime,
 * without calling parser trees structures and lists.
 */
typedef struct __runtime_conf_t { /*should be read-only for all*/
  host_config_t general; /*general settings*/
#ifdef VHOSTS
  usrtc_t *vhosts; /*search tree for vhosts if enabled*/
#endif
} runtime_conf_t;

/*configuration*/

struct variable {
  char *var;
  char *value;
};

char *get_general_value(const char *name);
struct variable *get_module_variables(const char *module);
struct variable *get_directory_variables(const char *directory);
struct variable *get_virtualhost_variables(const char *virtualhost);
struct variable *get_mimetype_variables(const char *typ);

/*----init-------*/
/*init default general values*/

/*----parser-----*/
/*read syntax and context tree*/
void load_configuration(char *buffer,int size);
void free_conf_tree(void);
int conf_walk_modulas(int (*found_hook)(struct variable *array,char *name));

#endif
