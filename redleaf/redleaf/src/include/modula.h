/*
 * RedLeaf modula_t structures, functions and defines
 *
 * Copyright (C) 2006, 2007, 2008 RedLeaf devteam org.
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

#ifndef __MODULA_H__
#define __MODULA_H__

#include <sys/types.h>

#include <http.h>
#include <serv.h>
#include <conf.h>

#define IMPLEMENTED      0
#define NOT_IMPLEMENTED  1

#define MOD_OPEN   0
#define MOD_CLOSE  1
#define MOD_READ   2
#define MOD_WRITE  3
#define MOD_SEEK   4

/*modula_info_t - modula information structure*/
typedef struct ___modula_info_t {
  char *name; /*modula's name*/
  char *author; /*modula's author*/
  char *version; /*version code*/
} modula_info_t;

/*modula_session_t - modula session describer*/
typedef struct ___modula_session_t {
  int pipe_rd; /*pipe read from*/
  int pipe_wr; /*pipe write to*/
  void *data; /*pointer to some data, if needed*/
  int (*modula_session_close)(struct ___modula_session_t *session); /*closes session*/
  size_t (*modula_session_read)(struct ___modula_session_t *session,void *buf,size_t size); /*read from pipe*/
  size_t (*modula_session_write)(struct ___modula_session_t *session,void *buf,size_t size); /*write to pipe*/
  size_t (*modula_session_seek)(struct ___modula_session_t *session,off_t offset); /*seek wothin output*/
} modula_session_t;

/*modula_t - main modula's astraction*/
typedef struct ___modula_t {
  modula_info_t info; /*generalized information*/
  char *registered_mime_type; /*registered mime type*/
  void *data; /*some data, if needed*/
  /*modula shoot*/
  int (*modula_init)(struct ___modula_t *modula,void *data); /*init modula specific things*/
  int (*modula_shootout)(struct ___modula_t *modula); /*kill, kill modula!*/
  /*modula session operations pointers*/
  /*create session*/
  int (*modula_session_open)(struct ___modula_t *modula,modula_session_t *session,struct http_request *htreq,void *data);
  int (*modula_session_close)(modula_session_t *session); /*close session*/
  size_t (*modula_session_read)(modula_session_t *session,void *buf,size_t size); /*related to modula_session_t pointers*/
  size_t (*modula_session_write)(modula_session_t *session,void *buf,size_t size);
  size_t (*modula_session_seek)(modula_session_t *session,off_t offset);
  /*misc functions*/
  int (*modula_check_capatibilies)(struct __modula_t *p,int op_code); /*check some capatibility*/
} modula_t;

#endif /*__MODULA_H__*/

