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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>

#include <conf.h>

/*TODO: fix the bugs, add additional checking*/
/*
 * FEATURE: add virtual hosting parsing and checking
 */
/*configuration parser and structures*/

#define MAX_SECTIONS  5

#define LINE_BUF  1024

#define __STRLEN(x) (((x) == NULL) ? 0 : strlen(x))
#define __STRDUP(x) (((x) == NULL) ? strdup("ERR") : strdup(x))


struct syn_tree {
  int order;
  char *name;
  char *argument;
  struct variable *vv;
  struct syn_tree *child; /*future reasons*/
  struct syn_tree *parent; /*future reasons*/
};

struct __conf_section {
  int order;
  char *name;
  char *arg;
  char *data;
};

typedef enum {
  _GENERAL,
  _MODULE,
  _DIRECTORY,
  _VIRTUALHOST,
} module_t;

/*static char *sections[]={"General","Module","Directory","VirtualHost"};
static char *general_pairs[]={"hostname","port","root_directory","log_level","user","group"};
static char *module_pairs[]={"path"};
static char *directory_pairs[]={"webpath","access","index"};
static char *virtualhost_pairs[]={"directory"};

static int general_tree_fit=5;*/
static unsigned int total_sections=0; 
static struct syn_tree local_tree[MAX_SECTIONS];

void init_defaults(void)
{
  return;
}

/*local functions prototypes*/
static int __scan_section_lines(char *buf);
static int __is_section_head_valid(char *buf);
static struct __conf_section __readwseek_section(char *buf,int order);
static int read_section(struct __conf_section *section,char *buf,int order);
static int __scan_line_expr(char *buf,int n);
static int read_syn_tree(char *buffer,int size);
/*static int __synlen(const char **src);*/
static struct variable *get_section_variables(const char *section,const char *name);

char *get_general_value(const char *name)
{
  struct syn_tree general;
  int i=0;

  while(i<=MAX_SECTIONS)
    if(!strncmp((const char*)local_tree[i].name,"General",__STRLEN("General"))){
      general=local_tree[i];
      break;
    } else i++;
  
  i=0;
  while(general.vv[i].var){
    if(!strcmp((const char*)general.vv[i].var,name))
      return general.vv[i].value;
    else i++;
  }

  return NULL;
}

struct variable *get_module_variables(const char *module)
{
  return get_section_variables("Module",module);
}

struct variable *get_directory_variables(const char *directory)
{
  return get_section_variables("Directory",directory);
}

struct variable *get_virtualhost_variables(const char *virtualhost)
{
  return get_section_variables("VirtualHost",virtualhost);
}

struct variable *get_mimetype_variables(const char *typ)
{
  return get_section_variables("Mime",typ);
}

void load_configuration(char *buffer,int size)
{

  if(read_syn_tree(buffer,size)==-1) {
    fprintf(stderr,"Error reading config file, using defaults.\n");
    //    __create_default_section();
  } else {
    //__check_local_tree_keywords();
  }
  return;
}

/*implementation*/
static int read_syn_tree(char *buffer,int size)
{
  char *buf=malloc(sizeof(char)*size),*tbuf=buffer,*fbuf=buf;
  int b_size=size;
  int ss=0,es=0,sl=0;
  struct __conf_section *cnf_section=NULL,*tsect;

  if(!buffer || !size) {
#ifdef _DEBUG_
    fprintf(stderr,"Couldn't read context tree with invalid parameters.\nAborting.\n");
#endif
    return -1;
  }
  if(!fbuf) {
#ifdef _DEBUG_
    fprintf(stderr,"Error allocating memory for buffer");
#endif
    return -1;
  }
  memset(buf,'\0',size);

  /*preprocess*/
  while(b_size){
    if(*tbuf=='#')  /*skip this line - comment line*/
      while(*tbuf!='\n' || b_size<=0){
        tbuf++;b_size--;
      }
    else if(*tbuf=='\n') { /*override*/
      tbuf++;b_size--;
    } else if(isblank((int)*tbuf)) { /*remove blanks*/
      tbuf++;b_size--;
    } else {
      *fbuf=*tbuf; tbuf++;b_size--;
      if(*fbuf=='{') ss++;
      if(*fbuf=='}') es++;
      fbuf++;
    }
  }

  if((ss-es)!=0) {
    fprintf(stderr,"Syntax error under '{|}'.\nAborting.\n");
    free(buf);
    return -1;
  }
  if(!ss || !es) {
    fprintf(stderr,"No sections are present.\nAborting.\n");
    free(buf);
    return -1;
  }

  fbuf=buf;es=0;
  cnf_section=malloc(sizeof(struct __conf_section)*ss);
  (*cnf_section).order=-1;
#ifdef _DEBUG_
  fprintf(stderr,"starting parsing ...\n");
  fprintf(stderr,"\'%s\'\n", fbuf);
#endif
  while(ss!=0) {
#ifdef _DEBUG_
    fprintf(stderr,"ss=%i..\n", ss);
#endif
    if(read_section(cnf_section,fbuf,es)!=0) {
      fprintf(stderr,"error: reading section (%d).\n",es);
      free(buf);
      return -1;
    } 
    fbuf+=sizeof(char)*(strlen(cnf_section[es].name)+strlen("{}()")+
        __STRLEN(cnf_section[es].arg)+__STRLEN(cnf_section[es].data));
    ss--;es++;
  }
  free(buf);

  ss=0;  tsect=cnf_section;
  total_sections=es;
  while(ss!=es){
    if(ss>MAX_SECTIONS) /*TODO:make reallocation after adding virtual hosts support*/
      break;
#ifdef _DEBUG_
    fprintf(stderr,"Section:\nName:%s;Argument:%s;Order:%d\nData:\n%s\n",
	    (*tsect).name,(*tsect).arg,(*tsect).order,(*tsect).data);
#endif
    local_tree[ss].name=__STRDUP((*tsect).name);
    local_tree[ss].argument=__STRDUP((*tsect).arg);
    local_tree[ss].order=(*tsect).order;
    fflush(stdout);
    if((*tsect).data) {
      char *expr=(*tsect).data;
      char *uui=expr;
      int i=0;
#ifdef _DEBUG_
      fprintf(stderr,"ee '%s'\n",(*tsect).data);
#endif
      sl=__scan_section_lines((*tsect).data);
      local_tree[ss].vv=NULL;
      if(__scan_line_expr((*tsect).data,sl)!=0)
        fprintf(stderr,"error: section has an error in expressions.\n");
      else {
        local_tree[ss].vv=malloc(sizeof(struct variable)*(sl+1));
        while(sl!=0){
          expr=uui;
          while(*uui!='=') uui++;
          *uui='\0';
          local_tree[ss].vv[i].var=strdup(expr);
#ifdef _DEBUG_
          fprintf(stderr,"var=%s\n",expr);
#endif
          uui++;expr=uui;
          while(*uui!=';') uui++;
          *uui='\0';
          local_tree[ss].vv[i].value=strdup(expr);
#ifdef _DEBUG_
          fprintf(stderr,"value=%s\n",expr);
#endif
          uui++;
          i++;sl--;
        }
        local_tree[ss].vv[i].var=NULL;
        free((*tsect).data);
      }
    }

    tsect++;ss++;
  }

  return 0;
}

static int __scan_line_expr(char *buf,int n)
{
  char *o=buf;int h=0;
  while(*o!='\0'){
    if(*o=='=') h++;
    o++;
  }
  return n-h;
}

static int read_section(struct __conf_section *section,char *buf,int order)
{
  struct __conf_section *t=section;
  char *Y=buf,T;

  while((*t).order!=-1)    t++; 

  while(*Y!='}') Y++;
  Y++;T=*Y;*Y='\0';

  *t=__readwseek_section(buf,order);
  *Y=T;
  buf=Y;

  if((*t).order==-2)
    return -1;
  t++;(*t).order=-1;  

  return 0;
}

static struct __conf_section __readwseek_section(char *buf,int order)
{
  char *u=buf;
  char *section_name,*section_argument;
  struct __conf_section sect;

  while(*u!='{')     u++;
  *u='\0';
  if(strlen(buf)<3){
    *u='{';
    fprintf(stderr,"error: invalid section name.\n");
    sect.order=-2;
    return sect;
  } 
  *u='{';u=buf;
  switch(__is_section_head_valid(buf)) {
    case 0:
      while(*buf!='(') buf++;
      *buf='\0';
      section_name=strdup(u); *buf='('; buf++;
      if(*buf==')') section_argument=NULL;
      else {
        u=buf;
        while(*buf!=')') buf++;
        *buf='\0';
        section_argument=strdup(u);
        *buf=')';
      }
      buf++;
      break;
    case -1:
      fprintf(stderr,"error: section defining is not valid.\n");
      sect.order=-2;
      return sect;
      break;
  }
  buf++;
  if(!__scan_section_lines(buf)){
    sect.order=order;
    sect.name=section_name;
    sect.arg=section_argument;
    sect.data=NULL;
    *buf+=sizeof(char)*2;
    return sect;
  } else {
    u=buf;
    while(*buf!='}') buf++;
    *buf='\0';
    sect.data=strdup(u);
    sect.name=section_name;
    sect.arg=section_argument;
    sect.order=order;
    *buf='}';buf++;
  }
  return sect;
}

static int __is_section_head_valid(char *buf)
{
  char *y=buf;
  int ss=0,es=0;

  while(*y!='\0') {
    if(*y=='(') ss++;
    if(*y==')') es++;
    y++;
  }
  if(!ss || !es)
    return -1;
  if((ss-es)!=0)
    return -1;
  if(ss>1 || es>1)
    return -1;

  return 0;
}

static struct variable *get_section_variables(const char *section,const char *name)
{
  int i=0;

  while(i<=MAX_SECTIONS)
    if(!strcmp((const char*)local_tree[i].name,section) && 
      !strcmp((const char*)local_tree[i].argument,name))
      return local_tree[i].vv;
    else i++;

  return NULL;
}

static int __scan_section_lines(char *buf)
{
  int l=0;
  char *u=buf;
  while(*u!='}' && *u!='\0') {
    if(*u==';') ++l;
    u++;
  }

  return l;
}

/*static int __synlen(const char **src)
{
  int i=0;

  while(src[i])    i++;

  return i;
  }*/
