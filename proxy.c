#include <stdio.h>
#include "csapp.h"
/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *conn_hdr = "Connection: close\r\n";
static const char *prox_hdr = "Proxy-Connection: close\r\n";
static const char *host_hdr_format = "Host: %s\r\n";
static const char *requestlint_hdr_format = "GET %s HTTP/1.0\r\n";
static const char *endof_hdr = "\r\n";

static const char *connection_key = "Connection";
static const char *user_agent_key= "User-Agent";
static const char *proxy_connection_key = "Proxy-Connection";
static const char *host_key = "Host";

void doit(int connfd);
void parse_uri(char *uri, char *hostname, char *path, int *port);
void build_http_header(char *http_header, char *hostname, char *path, int port, rio_t *client_rio);
int connect_endServer(char *hostname, int port, char *http_header);

int main(int argc, char **argv)
{
    int listenfd,connfd;
    socklen_t  clientlen;
    char hostname[MAXLINE],port[MAXLINE];

    struct sockaddr_storage clientaddr;/*generic sockaddr struct which is 28 Bytes.The same use as sockaddr*/

    if(argc != 2){
        fprintf(stderr,"usage :%s <port> \n",argv[0]);
        exit(1);
    }

    listenfd = Open_listenfd(argv[1]);
    while(1){
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

        /*print accepted message*/
        Getnameinfo((SA*)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
        printf("Accepted connection from (%s %s).\n",hostname,port);

        /*sequential handle the client transaction*/
        doit(connfd);

        Close(connfd);
    }
    return 0;
}

/*handle the client HTTP transaction*/
void doit(int connfd)
{
    int end_serverfd;/*the end server file descriptor*/

    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char endserver_http_header [MAXLINE];
    /*store the request line arguments*/
    char hostname[MAXLINE], path[MAXLINE];
    int port;

    rio_t rio,server_rio;/*rio is client's rio, server_rio is endserver's rio*/

    Rio_readinitb(&rio, connfd);
    Rio_readlineb(&rio, buf, MAXLINE);
    sscanf(buf,"%s %s %s", method, uri, version); /*read the client request line*/

    if(strcasecmp(method,"GET")){
        printf("Proxy does not implement the method");
        return;
    }
    /*parse the uri to get hostname,file path ,port*/
    parse_uri(uri, hostname, path, &port);  // uri 를 ':'와 '/' 로 나누어 세 곳에 집어 넣음

    /*build the http header which will send to the end server*/
    build_http_header(endserver_http_header, hostname, path, port, &rio); // 보낼 헤더 받고 마련하기

    /*connect to the end server*/
    end_serverfd = connect_endServer(hostname, port, endserver_http_header);  // 프록시에서 끝 서버로 보내어 연결할 소켓 생성
    if(end_serverfd<0){
        printf("connection failed\n");
        return;
    }

    Rio_readinitb(&server_rio, end_serverfd); // 서버로 보낼 rio 구조체

    /*write the http header to endserver*/
    Rio_writen(end_serverfd, endserver_http_header, strlen(endserver_http_header));

    /*receive message from end server and send to the client*/
    size_t n;
    while((n=Rio_readlineb(&server_rio, buf, MAXLINE))!=0)
    {
        printf("proxy received %ld bytes, then send\n",n);
        Rio_writen(connfd, buf,n);
    }
    Close(end_serverfd);
}

void build_http_header(char *http_header, char *hostname, char *path, int port, rio_t *client_rio)
{
    char buf[MAXLINE], request_hdr[MAXLINE], other_hdr[MAXLINE], host_hdr[MAXLINE];
    /*request line*/
    sprintf(request_hdr, requestlint_hdr_format, path); // 준비한 포맷으로 첫줄 request_hdr에 저장
    /*get other request header for client rio and change it */
    while(Rio_readlineb(client_rio, buf, MAXLINE)>0)
    {
        if(strcmp(buf, endof_hdr)==0) break;/*헤더가 공백으로 끝나면 반복 종료*/

        if(!strncasecmp(buf, host_key, strlen(host_key)))/*Host:*/
        {
            strcpy(host_hdr, buf);    // "HOST" 로 들어오는 헤더가 있다면 버퍼에 집어넣고 다음줄로 넘어가기
            continue;
        }

        if(!strncasecmp(buf,connection_key,strlen(connection_key))
                &&!strncasecmp(buf,proxy_connection_key,strlen(proxy_connection_key))
                &&!strncasecmp(buf,user_agent_key,strlen(user_agent_key)))
        {
            strcat(other_hdr,buf);  // 다른 설정해둔 포맷으로 들어오면 other_hdr 변수에 이어 붙이기
        }
    }
    if(strlen(host_hdr)==0)
    {
        sprintf(host_hdr, host_hdr_format, hostname); // HOST 시작 헤더 안들어오면 그냥 기존 uri에서저장해둔 hostname으로 대체
    }
    sprintf(http_header,"%s%s%s%s%s%s%s",
            request_hdr,
            host_hdr,
            conn_hdr,
            prox_hdr,
            user_agent_hdr,
            other_hdr,
            endof_hdr);   // http_header 위 내용들 다 합쳐 저장

    return ;
}
/*Connect to the end server*/
inline int connect_endServer(char *hostname,int port,char *http_header){
    char portStr[100];
    sprintf(portStr,"%d", port);
    return Open_clientfd(hostname, portStr);
}

/*parse the uri to get hostname,file path ,port*/
void parse_uri(char *uri,char *hostname,char *path,int *port)
{
    *port = 80;
    char* pos = strstr(uri, "//");

    pos = pos!=NULL? pos+2:uri; // 아무튼 pos 가 uri

    char*pos2 = strstr(pos, ":");
    if(pos2!=NULL)
    {
        *pos2 = '\0';   // pos2 가 "//" 이후 uri 의 : 를 가르켰으니, 이를 null로 바꾸면 포트번호 전 후로 나눔
        sscanf(pos,"%s", hostname);  // hostname(ip) 은 포트번호 전
        if(strstr(pos2+1, "/")!=NULL){  // '/'로 끝났다면
          sscanf(pos2+1, "%d%s", port, path); // port 엔 포트번호, path 엔 그뒤의 '/' 부터의 경로
        }
        else  // '/'없이 들어왔다면
        {
          sscanf(pos2+1, "%d", port);   // 포트번호만 적고
          path = "/";
        }

    }
    else  // : 가 없다면
    {
        pos2 = strstr(pos, "/");
        if(pos2!=NULL)
        {
            *pos2 = '\0';
            sscanf(pos,"%s",hostname);    // '/' 전이 hostname
            *pos2 = '/';
            sscanf(pos2,"%s",path);   // 그 뒤가 path
        }
        else  // ':' 도 없고 '/' 도 없다면.
        {
            sscanf(pos,"%s",hostname);  // 그냥 바로 pos 가 hostname
        }
    }
    return;
}