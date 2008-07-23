/*
 * RedLeaf HTTP 1.1 implementation
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

/*basic HTTP implementation parser*/

#ifndef __HTTP_H__
#define __HTTP_H__

/*HTTP errors*/
/*informational 1xx*/
#define CONTINUE             100
#define SWITCHING_PROTOCOLS  101
#ifdef WEBDAV
#define PROCESSING           102
#endif
/*Success 2xx*/
#define OK                             200
#define CREATED                        201
#define ACCEPTED                       202
#define NON_AUTHORITATIVE_INFORMATION  203
#define NO_CONTENT                     204
#define RESET_CONTENT                  205
#define PARTIAL_CONTENT                206
#ifdef WEBDAV
#define MULTI_STATUS                   207
#endif
/*Redirection 3xx*/
#define MULTIPLY_CHOICES     300
#define MOVED_PERMANENTLY    301
#define FOUND                302
#define SEE_OTHER            303
#define NOT_MODIFIED         304
#define USE_PROXY            305
#define SWITCH_PROXY         306
#define TEMPRORARY_REDIRECT  307
/*Client error 4xx*/
#define BAD_REQUEST                      400
#define UNAUTHORIZED                     401
#define PAYMENT_REQUERED                 402
#define FORBIDDEN                        403
#define NOT_FOUND                        404
#define METHOD_NOT_ALLOWED               405
#define NOT_ACCEPTABLE                   406
#define PROXY_AUTHENTICATION_REQUERED    407
#define REQUEST_TIMEOUT                  406
#define CONFLICT                         409
#define GONE                             410
#define LENGTH_REQUERED                  411
#define PRECONDITION_FAILED              412
#define REQUEST_ENTITY_TOO_LARGE         413
#define REQUEST_URI_TOO_LONG             414
#define UNSUPPORTED_MEDIA_TYPE           415
#define REQUESTED_RANGE_NOT_SATISFIABLE  416
#define EXPECTATION_FAILED               417
#ifdef WEBDAV
#define UNPROCESSABLE_ENTITY             422
#define LOCKED                           423
#define FAILED_DEPENDENCY                424
#define UNORDERED_COLLECTION             425
#endif
#define UPGRADE_REQUIRED                 426 /*(RFC 2817)*/
/*Server error 5xx*/
#define INTERNAL_SERVER_ERROR       500
#define NOT_IMPLEMENTED             501
#define BAD_GATEWAY                 502
#define SERVICE_UNAVIALABLE         503
#define GATEWAY_TIMEOUT             504
#define HTTP_VERSION_NOT_SUPPORTED  505
#ifdef WEBDAV
#define INSUFFICIENT_STORAGE        507
#endif
#define BANDWIDTH_LIMIT_EXCEEDED    509

/*date format*/
#define RFC1123FMT  "%a, %d %b %Y %H:%M:%S GMT"

/*TODO: add unsupported request*/
typedef enum {
  GET,
  POST,
} request_t;

typedef enum {
  CLOSED,
  KEEP_ALIVE,
} connection_t;

/*structure used for http request*/
struct http_request {
  request_t method;
  char *uri;
  char *host;
  char *user_agent;
  char *accept;
  char *accept_language;
  char *accept_encoding;
  char *accept_charset;
  char *referer;
  char *cookie;
  size_t range;
  int keep_alive;
  connection_t connection_type;
  int op_code; /*parse error if exist*/
};

struct http_response {
  char *response;
  char *server;
  char *content_type;
  unsigned long content_lenght;
};

struct http_reply {
  char *head;
  char *fmtdate;
  char *server;
  char *content_type;
  char *connection_type;
  unsigned long content_length;
  void *buf; /*contents of the reply*/
  char *uri;
};

/*functions*/
struct http_request *parse_http_request(const char *msg);
struct page_t *page_t_generate(const char *request);
void free_http_request(struct http_request *r);

#endif /*__HTTP_H__*/
