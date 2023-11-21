#include <stdio.h>

#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE  1049000
#define MAX_OBJECT_SIZE 102400

#define METHOD_LEN  16
#define VERSION_LEN 16
#define PORT_LEN    32

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";

static void proxy(int);
static void parse_request(int, struct Request);

struct Request {
    char request[MAXLINE];
    char method[METHOD_LEN];    // GET
    char host_addr[MAXLINE];    // www.cmu.edu
    char port[PORT_LEN];        // default port: 80
    char path[MAXLINE];         // /hub/indexl.html
    char version[VERSION_LEN];  // HTTP/1.1
};

int main(int argc, char **argv) {
    int clientfd, listenfd, connfd = NULL;
    char hostname[MAXLINE], port[MAXLINE];
    rio_t rio;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    /* Check command line args */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    listenfd = Open_listenfd(argv[1]);

    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr,
                        &clientlen);  // line:netp:tiny:accept
        Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port,
                    MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);

        proxy(connfd);
    }
    Close(connfd);

    return 0;
}

void proxy(int clientfd) {
    int serverfd, is_static;
    char buf_ser[MAXLINE], buf_cli[MAXLINE];
    char uri[MAXLINE], filename[MAXLINE], cgiargs[MAXLINE];
    char default_port = "80";
    rio_t rio_client, rio_server;
    struct Request *req;
    struct stat sbuf;

    req = (struct Request *)malloc(sizeof(struct Request));

    parse_request(clientfd, req);

    // send req to server
    if (req->method == "GET") {
        static char buf[MAXLINE];
        static char *request[MAXLINE];


    }
}

void parse_request(int clientfd, struct Request *req) {
    int is_static;
    char buf[MAXLINE];
    char uri[MAXLINE], filename[MAXLINE], cgiargs[MAXLINE];
    char default_port = "80";
    rio_t rio;
    struct stat sbuf;

    Rio_readinitb(&rio, clientfd);

    if (Rio_readlineb(&rio, buf, MAXLINE) > 0) {
        printf("Request headers: \n");
        printf("%s", buf);
        strncpy(req->request, buf, strlen(buf));
        sscanf(buf, "%s %s %s", req->method, uri, req->version);
    }

    // uri : http://csapp.cs.cmu.edu:80/index.html
    // hostname: csapp.cs.cmu.edu
    // hostname_port: 80
    // path: /index.html

    char hostname_port[MAXLINE];  // this is temp char for hostname and port
    char path[MAXLINE];
    char port[PORT_LEN];  // hostname_port is later divided into port and hostname
    char hostname[MAXLINE];
    char *uri_pt = uri;

    uri_pt = strstr(uri_pt, "://");

    // uri : http://csapp.cs.cmu.edu:80/index.html
    //              ^
    //           uri_pt
    uri_pt = uri_pt + strlen("://");
    path[0] = '/';
    //"%[^/]" indicates the string can contain any charater other than '/'
    sscanf(uri_pt, "%[^/]%s", hostname_port, path);
    //"%[^:]" indicates the string can contain any charater other than ':'
    sscanf(hostname_port, "%[^:]:%s", hostname, port);

    strncpy(req->host_addr, hostname, strlen(hostname));
    strncpy(req->path, path, strlen(path));

    if (strlen(port) == 0) {
        strcpy(req->port, default_port);
    } else {
        strncpy(req->port, port, strlen(port));
    }
    printf("parse request done\n");
}