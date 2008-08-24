/*
 * RedLeaf IPC part
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

#ifndef __SERV_H__
#define __SERV_H__

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <time.h>

#include "../../config.h"

#include <page.h>
#include <file.h>

#define RD_TIMEOUT  30
#define MAXBUF_LEN  1024

typedef enum {
  ST_READ,
  ST_PRCS,
  ST_NONE,
  ST_TIMEOUT,
  ST_ERROR,
  ST_DONE,
} state_t;

typedef enum {
  SS_HEAD,
  SS_BODY,
  SS_FILE,
#ifdef MODULAS
  SS_MODULA,
#endif
  SS_DONE,
} hbs_t;

struct connection_t {
  int socket;
  struct in_addr addr;

  /*states*/
  unsigned short rxstat,wxstat;

  /*timing*/
  time_t last_state;
  time_t elapsed_time;

  /*counting*/
  size_t total_size; /*total bytes to send*/
  size_t sent_size; /*sent bytes (head+body)*/

  size_t request_len;
  char *request;
  char *req_ptr;

  size_t data_len;
  void *data;
  void *data_ptr;

  hbs_t hb_switch;

  void *data_send;

  /*page for process*/
  struct page_t *page;
  /*file session if exist*/
  struct file_session_t *file;
  /*http session*/
  http_session_t *http;
};

/*global functions*/

int main_process(int argc,char **argv);

#endif
