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

/*structure used for http request*/
struct http_request {
  char *method; /*method of request*/
  char *location; /*location*/
  char *pver; /*protocol version*/
  char **vars; /*variables list*/
  char **values; /*values list*/
  int vlist; /*number of variables*/
  int op_code; /*parse error if exist*/
};

struct http_response {
  char *response;
  char *server;
  char *content_type;
  unsigned long content_lenght;
};

/*functions*/
struct http_request *parse_http_request(char *msg);
int process_request(struct http_request *r,int fd);

#endif /*__HTTP_H__*/
