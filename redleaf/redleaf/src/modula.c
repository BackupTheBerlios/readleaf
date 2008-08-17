/*
 * RedLeaf configuration manager 
 *
 * Copyright (C) 2008 RedLeaf devteam org.
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>
#include <dlfcn.h>

#include <conf.h>
#include <misc.h>
#include <modula.h>
#include <http.h>
#include <libdata/usrtc.h>

static int _is_modulas=0;
usrtc_t *modulas;

static int compare_function(const void *a, const void *b);
static int modula_load_hook(struct variable *vv,char *name);

int init_modulas_subsystem(void) 
{
  modulas=usrtc_create(USRTC_SPLAY,65535,compare_function);
  conf_walk_modulas(modula_load_hook);

  return 0;
}

modula_t *modula_lookup(char *mime_type)
{
  usrtc_node_t *emod=NULL;

  if(!_is_modulas)
    return NULL;

  emod=usrtc_lookup(modulas,(void *)mime_type);
  if(emod)
    return (modula_t *)usrtc_node_getdata(emod);

  return NULL;
}

static int modula_load_hook(struct variable *vv,char *name)
{
  int i=0;
  modula_t *mod=rl_malloc(sizeof(modula_t));
  usrtc_node_t *emod;
  char *mod_path=NULL,*reg_mime=NULL,*func_name=NULL;
  const char *err;
  void *handle;
  /*newly registered functions*/
  int (*modula_init)(modula_t *mod,void *data);
  int (*modula_shootout)(modula_t *mod);
  int (*modula_check_capatibilies)(int op_code);

  if(!vv || !name) {
    fprintf(stderr,"Cannot load modula due to possible config errors.\n");
    rl_free(mod);
    return -1;
  }

  while(vv[i].var) {
    if(!strcmp((const char*)vv[i].var,"modula_path")) 
      mod_path=vv[i].value;
    else if(!strcmp((const char*)vv[i].var,"registered_mime"))
      reg_mime=vv[i].value;
    if(mod_path && reg_mime)
      break;
    i++;   
  }
  handle=dlopen(mod_path,RTLD_LAZY);
  if(!handle) {
    perror("dlopen: ");
    rl_free(mod);
    return -1;
  }
  i=strlen(name)+strlen("_init")+2;
  func_name=rl_malloc(256);
  memset(func_name,'\0',256);
  snprintf(func_name,i,"%s_init",name);
  modula_init=dlsym(handle,func_name);
  if((err=dlerror())!=NULL) {
  _err_close:
    fprintf(stderr,"dlsym: %s\n",err);
    rl_free(mod);
    dlclose(handle);
    return -1;
  }

  modula_init(mod,vv); /*register it*/
  mod->cname=name;
  mod->registered_mime_type=reg_mime;
  mod->modula_init=modula_init;
  /*shootout*/
  memset(func_name,'\0',256);
  snprintf(func_name,255,"%s_shootout",name);
  modula_shootout=dlsym(handle,func_name);
  if((err=dlerror())!=NULL) 
    goto _err_close;
  mod->modula_shootout=modula_shootout;
  /*before full init, we're need to pick up capatibilities*/
  memset(func_name,'\0',256);
  snprintf(func_name,255,"%s_check_capatibilies",name);
  modula_check_capatibilies=dlsym(handle,func_name);
  if((err=dlerror())!=NULL) 
    goto _err_close;
  mod->modula_check_capatibilies=modula_check_capatibilies;
  /*ok, now attach other funcs(session related)*/
  for(i=0;i<5;i++) 
    if(modula_check_capatibilies(i)!=ABSENT) {
      memset(func_name,'\0',256);
      switch(i) {
      case MOD_OPEN:
	snprintf(func_name,255,"%s_session_open",name);
	mod->modula_session_open=dlsym(handle,func_name);
	if((err=dlerror())!=NULL) 
	  goto _err_close;
	break;
      case MOD_CLOSE:
	snprintf(func_name,255,"%s_session_close",name);
	mod->modula_session_close=dlsym(handle,func_name);
	if((err=dlerror())!=NULL) 
	  goto _err_close;
	break;
      case MOD_READ:
	snprintf(func_name,255,"%s_session_read",name);
	mod->modula_session_read=dlsym(handle,func_name);
	if((err=dlerror())!=NULL) 
	  goto _err_close;
	break;
      case MOD_WRITE:
	snprintf(func_name,255,"%s_session_write",name);
	mod->modula_session_write=dlsym(handle,func_name);
	if((err=dlerror())!=NULL) 
	  goto _err_close;
	break;
      case MOD_SEEK:
	snprintf(func_name,255,"%s_session_seek",name);
	mod->modula_session_seek=dlsym(handle,func_name);
	if((err=dlerror())!=NULL) 
	  goto _err_close;
	break;
      }
    }

  /*insert*/
  _is_modulas++;
  emod=usrtc_node_create((void *)mod); 
  usrtc_node_setdata(emod,(void *)mod);
  usrtc_insert(modulas,emod,(void *)reg_mime);

  return 0;
}

static int compare_function(const void *a, const void *b)
{
  return strcmp((const char *)a,(const char *)b);
}

