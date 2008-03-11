/*
 * RedLeaf main entry
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <http.h>
#include <serv.h>
#include <conf.h>
#include <page.h>
#include <misc.h>

int main(int argc, char **argv)
{
  int size;
  char *buf=NULL;

  buf=(char*)mmap_file("example.conf",&size);
  load_configuration(buf,size);
  init_page_t_cache();
  main_process(argc,argv);

  return 0;
}
