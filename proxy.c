#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *ip, char *port, char *path);
void connect_origin_server(int fd, char *buf, char *host, char *port, char *path);
void *thread(void *vargp);

int main(int argc, char **argv) 
{
    int listenfd, *connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;

    if (argc != 2) {
	fprintf(stderr, "usage: %s <port>\n", argv[0]);
	exit(1);
    }

    listenfd = Open_listenfd(argv[1]); // 프록시 소켓 오픈
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Malloc(sizeof(int));
        *connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); // (to client) 원본 서버로의 연결 요청
        Pthread_create(&tid, NULL, thread, connfd);
        // Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0); // (to client) 원본 서버 연결 정보 받음
        // printf("Accepted connection from (%s, %s)\n", hostname, port);
        // doit(connfd); // 트랜잭션 수행
        // Close(connfd); // 연결 종료
    }
}

// HTTP 트랜잭션 처리
void doit(int fd) 
{
    int is_static;
    struct stat sbuf;
    char host[MAXLINE], port[MAXLINE], path[MAXLINE];
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    rio_t rio;

    /* Read request line and headers */
    Rio_readinitb(&rio, fd);
    if (!Rio_readlineb(&rio, buf, MAXLINE)) return;  // request를 읽는다.
    // printf("Request headers : \n"); 
    printf("%s", buf);
    
    /* telnet 으로 들어온 정보 Example : GET http://localhost:5000/home.html HTTP/1.1 */
    sscanf(buf, "%s %s %s", method, uri, version); //method, uri, version 정보를 입력받는다.
    
    if (strcasecmp(method, "GET")) { // request method 종류가 GET인지 확인
        printf("501 Not Implemented, Proxy does not implement this method");
        return;
    }
    read_requesthdrs(&rio); //요청 헤더를 읽어 들인다.

    /* URI를 분리한다. Parse URI from GET request */
    parse_uri(uri, host, port, path);
    connect_origin_server(fd, buf, host, port, path);
}

/* Thread routine */
void *thread(void *vargp) 
{
    int connfd = *((int *)vargp);
    Pthread_detach(pthread_self());   
    Free(vargp);
    doit(connfd);
    Close(connfd);
    return NULL;
}
/*
 * read_requesthdrs - read HTTP request headers
 */
void read_requesthdrs(rio_t *rp) 
{
    char buf[MAXLINE];

    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf);
    while(strcmp(buf, "\r\n")) {
        Rio_readlineb(rp, buf, MAXLINE);
        printf("%s", buf);
    }
    return;
}

int parse_uri(char *uri, char *host, char *port, char *path)
{
    char *extracturi;
    char *p;

    /* uri 파싱 */
    if(strstr(uri, "http://") != NULL) { // "http://" 가 uri에 존재하는 경우
        extracturi = uri + 7;
        printf("extracturi : %s\n", extracturi); // 지워야함        
    }
    p = strstr(extracturi, "/");
    strcpy(path, p);
    strncpy(host, extracturi, p - extracturi);
    *(host + (p - extracturi)) = '\0'; // strncpy_s 와 같은 기능 

    p = strstr(host, ":") + 1;
    strcpy(port, p);
    strncpy(host, extracturi, p - host - 1);
    *(host + (p - host - 1)) = '\0'; // strncpy_s 와 같은 기능 
    printf("ip : %s / port : %s / path : %s \n", host, port, path);
}

void connect_origin_server(int listenfd, char *buf, char *host, char *port, char *path)
{
    int fd;
    rio_t rio;
    size_t n;

    // printf("buf content : %s", buf);
    fd = Open_clientfd(host, port); // 원본 서버와 연결
    Rio_readinitb(&rio, fd);

    /* request header 생성 */
    sprintf(buf, "GET %s HTTP/1.0\r\n", path);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Host: %s\r\n", host);
    Rio_writen(fd, buf, strlen(buf)); 
    sprintf(buf, "%s", user_agent_hdr); 
    Rio_writen(fd, buf, strlen(buf)); 
    sprintf(buf, "Connection: close\r\n");
    Rio_writen(fd, buf, strlen(buf)); 
    sprintf(buf, "Proxy-Connection: close\r\n");
    Rio_writen(fd, buf, strlen(buf)); 
    sprintf(buf, "\r\n");
    Rio_writen(fd, buf, strlen(buf));
    
    while((n = Rio_readlineb(&rio,buf,MAXLINE))!=0)
    {
        printf("proxy received %d bytes,then send\n",n);
        Rio_writen(listenfd,buf,n);
    }
    Close(fd); 
}
