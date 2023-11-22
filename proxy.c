#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE  1049000
#define MAX_OBJECT_SIZE 102400

#define METHOD_LEN  16
#define VERSION_LEN 16
#define PORT_LEN    32

struct Request {
    char request[MAXLINE];
    char method[METHOD_LEN];
    char host_addr[MAXLINE];
    char port[PORT_LEN];
    char path[MAXLINE];
    char version[VERSION_LEN];
};

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";

static void proxy(int);
static void parse_request(int, struct Request *);
static void send_req_to_server(int, struct Request *);
static int send_res_to_client(int, int);

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
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port,
                    MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);

        proxy(connfd);
    }

    Close(connfd);
    return 0;
}

void proxy(int clientfd) {
    int is_static;
    char buf_ser[MAXLINE], buf_cli[MAXLINE];
    char uri[MAXLINE], filename[MAXLINE], cgiargs[MAXLINE];
    char default_port = "80";
    rio_t rio;
    struct stat sbuf;

    struct Request *req = (struct Request *)malloc(sizeof(struct Request));

    parse_request(clientfd, req);

    int serverfd = Open_clientfd(req->host_addr, req->port);

    send_req_to_server(serverfd, req);

    free(req);

    send_res_to_client(clientfd, serverfd);
}

static void parse_request(int clientfd, struct Request *req) {
    int is_static;
    char buf[MAXLINE];
    char uri[MAXLINE], filename[MAXLINE], cgiargs[MAXLINE];
    char default_port = "80";
    rio_t rio;
    struct stat sbuf;

    Rio_readinitb(&rio, clientfd);

    if (Rio_readlineb(&rio, buf, MAXLINE) > 0) {
        char *null_ptr = strstr(buf, "\n");
        *null_ptr = '\0';
        strcpy(req->request, buf);

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

    strcpy(req->host_addr, hostname);
    strcpy(req->path, path);

    if (strlen(port) == 0) {
        strcpy(req->port, default_port);
    } else {
        strncpy(req->port, port, strlen(port));
    }
}

static void send_req_to_server(int serverfd, struct Request *req) {
    // send req to server
    if (strstr(req->method, "GET") != 0) {
        static char buf[MAXLINE];
        sprintf(buf, "%s %s %s\r\n", req->method, req->path, req->version);
        Rio_writen(serverfd, buf, strlen(buf));
        sprintf(buf, "HOST: %s\r\n", req->host_addr);
        Rio_writen(serverfd, buf, strlen(buf));
        sprintf(buf, "%s", user_agent_hdr);
        Rio_writen(serverfd, buf, strlen(buf));
        sprintf(buf, "Connection: close\r\n");
        Rio_writen(serverfd, buf, strlen(buf));
        sprintf(buf, "Proxy-Connection: close\r\n\r\n");
        Rio_writen(serverfd, buf, strlen(buf));
    }
}

static int send_res_to_client(int clientfd, int serverfd) {
    int buffer_size;
    socklen_t option_len = sizeof(buffer_size);

    if (getsockopt(serverfd, SOL_SOCKET, SO_RCVBUF, &buffer_size, &option_len) < 0) {
    perror("getsockopt error");
    // 에러 처리
    } else {
        printf("Receive buffer size: %d\n", buffer_size);
    }

    char *buf = (char *)malloc(buffer_size);
    char *res = (char *)malloc(buffer_size);
    
    res[0] = '\0';
    int res_len = 0;

    int n = rio_readn(serverfd, buf, buffer_size);

    buf[buffer_size] = '\0';

    if (res_len < MAX_OBJECT_SIZE) {
        res_len += n;
        if (res_len < MAX_OBJECT_SIZE) {
            strcat(res, buf);
        }
        else {
            res_len = MAX_OBJECT_SIZE;
        }
    }

    if (rio_writen(clientfd, buf, n) < 0) {return -1;}
    return res_len;
}