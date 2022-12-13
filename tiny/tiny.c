/* $begin tinymain */
/*
 * tiny.c - A simple, iterative HTTP/1.0 Web server that uses the 
 *     GET method to serve static and dynamic content.
 */
#include "csapp.h"

static char reshttp[MAXLINE] = "1.0"; // 연습 문제 6-c
void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize, char *method);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs, char *method);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);

int main(int argc, char **argv) 
{
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    /* Check command line args */
    if (argc != 2) {
	fprintf(stderr, "usage: %s <port>\n", argv[0]);
	exit(1);
    }

    listenfd = Open_listenfd(argv[1]); // 서버쪽 듣기 소켓을 연다.
    while (1) { // 무한 루프 실행
	clientlen = sizeof(clientaddr);
	connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); // 연결 요청 접수
        Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, 
                    port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
	doit(connfd);                                             // 트랜잭션 수행
	Close(connfd);                                            // 연결 종료
    }
}
/* $end tinymain */

/*
 * doit - 한 개의 HTTP 트랜잭션을 처리한다.
 * handle one HTTP request/response transaction
 */
/* $begin doit */
void doit(int fd) 
{
    int is_static;
    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];
    rio_t rio;

    /* Read request line and headers */
    Rio_readinitb(&rio, fd);
    if (!Rio_readlineb(&rio, buf, MAXLINE)) return;  // request를 읽는다. doit:readrequest
    printf("Request headers : \n"); 
    printf("%s", buf);
    
    sscanf(buf, "%s %s %s", method, uri, version);       //method, uri, version 정보를 입력받는다. doit:parserequest
    
    // 연습 문제 11.11 - HEAD method 추가
    if (strcasecmp(method, "GET") && strcasecmp(method, "HEAD")) { //request method 종류가 GET인지 비교한다. doit:beginrequesterr
        clienterror(fd, method, "501", "Not Implemented",
                    "Tiny does not implement this method");
        return;
    }                                                    //doit:endrequesterr
    read_requesthdrs(&rio); //요청 헤더를 읽어 들인다. doit:readrequesthdrs

    /* URI를 분리한다. Parse URI from GET request */
    is_static = parse_uri(uri, filename, cgiargs);       //요청이 정적 콘텐츠인지 동적 콘텐츠인지 판단한다. doit:staticcheck
    if (stat(filename, &sbuf) < 0) {                     //파일이 디스크 상에 존재하지 않을 때. doit:beginnotfound
	clienterror(fd, filename, "404", "Not found",
		    "Tiny couldn't find this file");
	return;
    }                                                    //doit:endnotfound


    /* 1. 정적 콘텐츠 일 때. Serve static content */          
    if (is_static) {
        //보통 파일이면서 읽기 권한 존재 확인. doit:readable
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
            clienterror(fd, filename, "403", "Forbidden",
                "Tiny couldn't read the file");
            return;
        }
        //정적 컨텐츠를 사용자에게 제공한다. doit:servestatic
        serve_static(fd, filename, sbuf.st_size, method);       
    }
    /* 2. 동적 콘텐츠 일 때. Serve dynamic content */
    else { 
    //보통 파일이면서 실행 권한 존재 확인. doit:executable(동적 콘텐츠이므로 실행 권한을 확인한다.)
	if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) { 
	    clienterror(fd, filename, "403", "Forbidden",
			"Tiny couldn't run the CGI program");
	    return;
	}
    //동적 컨텐츠를 사용자에게 제공한다. doit:servedynamic
	serve_dynamic(fd, filename, cgiargs, method);
    }
}
/* $end doit */

/*
 * read_requesthdrs - read HTTP request headers
 */
/* $begin read_requesthdrs */
void read_requesthdrs(rio_t *rp) 
{
    char buf[MAXLINE];

    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf);
    while(strcmp(buf, "\r\n")) {          //readhdrs:checkterm
        Rio_readlineb(rp, buf, MAXLINE);
        printf("%s", buf);
    }
    return;
}
/* $end read_requesthdrs */

/*
 * parse_uri - parse URI into filename and CGI args
 *             return 0 if dynamic content, 1 if static
 */
/* $begin parse_uri */
int parse_uri(char *uri, char *filename, char *cgiargs) 
{
    char *ptr;

    /* 정적 콘텐츠 요청. Static content */
    if (!strstr(uri, "cgi-bin")) { //parseuri:isstatic
	strcpy(cgiargs, ""); //cgi인자 스트링을 지운다.(존재 x) parseuri:clearcgi
	strcpy(filename, "."); //상대 리눅스 경로 이름으로 변환한다. parseuri:beginconvert1
	strcat(filename, uri); //parseuri:endconvert1
	if (uri[strlen(uri)-1] == '/') //만약 URI가 '/'로 끝난다면 기본 파일 이름을 추가한다. parseuri:slashcheck
	    strcat(filename, "home.html"); //parseuri:appenddefault
	return 1;
    }
    
    /* 동적 컨텐츠 요청. Dynamic content */
    else { //parseuri:isdynamic
    //cgi 인자들을 추출한다. parseuri:beginextract
	ptr = index(uri, '?'); 
	if (ptr) {
	    strcpy(cgiargs, ptr+1);
	    *ptr = '\0';
	}
	else 
	    strcpy(cgiargs, "");
    //cgi 인자 추출 완료. parseuri:endextract
	strcpy(filename, "."); //상대 리눅스 경로 이름으로 변환한다. parseuri:beginconvert2
	strcat(filename, uri); //parseuri:endconvert2
	return 0;
    }
}
/* $end parse_uri */

/*
 * clienterror - returns an error message to the client
 */
/* $begin clienterror */
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg) 
{
    char buf[MAXLINE], body[MAXBUF];

    /* Build the HTTP response body */
    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

    /* Print the HTTP response */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, strlen(body));
}
/* $end clienterror */


/*
 * serve_static - copy a file back to the client 
 */
/* $begin serve_static */
void serve_static(int fd, char *filename, int filesize, char *method) 
{
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXBUF];
    rio_t rio;
 
    /* Send response headers to client */
    get_filetype(filename, filetype); //파일 이름을 통해 파일 타입 결정. servestatic:getfiletype
    sprintf(buf, "HTTP/%s 200 OK\r\n", reshttp); //헤더 전송. servestatic:beginserve
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    sprintf(buf, "%sConnection: close\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    Rio_writen(fd, buf, strlen(buf)); //헤더 종료. servestatic:endserve
    printf("Response headers:\n");
    printf("%s", buf);

    if(strcasecmp(method, "GET") == 0) { // method가 GET일때만 응답 객체 받게끔
        /* 요청한 파일의 내용을 fd로 복사해서 응답 본체를 보낸다. Send response body to client */
        srcfd = Open(filename, O_RDONLY, 0); //읽기 위해 filename을 오픈한다. servestatic:open
        srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);//요청한 파일을 가상 메모리 영역으로 매핑한다. servestatic:mmap
        Close(srcfd); // 파일을 메모리로 매핑한 후 더 이상 식별자가 필요 없으므로 파일을 닫는다. servestatic:close
        Rio_writen(fd, srcp, filesize);         //servㅁestatic:write
        Munmap(srcp, filesize);                 //servestatic:munmap
    }

    /* 연습 문제 11.9
    srcfd = Open(filename, O_RDONLY, 0); //읽기 위해 filename을 오픈한다. servestatic:open
    
    srcp = (char *)malloc(filesize);
    Rio_readn(srcfd, srcp, filesize);
    Close(srcfd); // 파일을 메모리로 매핑한 후 더 이상 식별자가 필요 없으므로 파일을 닫는다. servestatic:close
    Rio_writen(fd, srcp, filesize);         //servestatic:write
    free(srcp);
    */
}

/*
 * get_filetype - derive file type from file name
 */
void get_filetype(char *filename, char *filetype) 
{
    if (strstr(filename, ".html"))
	strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
	strcpy(filetype, "image/gif");
    else if (strstr(filename, ".png"))
	strcpy(filetype, "image/png");
    else if (strstr(filename, ".jpg"))
	strcpy(filetype, "image/jpeg");
    else if (strstr(filename, ".mpg"))
    strcpy(filetype, "video/mpg"); // 연습 문제 11.7 
    else
	strcpy(filetype, "text/plain");
}  
/* $end serve_static */

/*
 * serve_dynamic - run a CGI program on behalf of the client
 */
/* $begin serve_dynamic */
void serve_dynamic(int fd, char *filename, char *cgiargs, char *method) 
{
    char buf[MAXLINE], *emptylist[] = { NULL };

    /* Return first part of HTTP response */
    sprintf(buf, "HTTP/1.0 200 OK\r\n"); 
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Server: Tiny Web Server\r\n");
    Rio_writen(fd, buf, strlen(buf));
  
    /* Child */ //새로운 자식 프로세스를 fork 한다. servedynamic:fork
    if (Fork() == 0) { 
	/* Real server would set all CGI vars here */
	setenv("QUERY_STRING", cgiargs, 1); //QUERY_STRING 환경 변수를 요청 URI의 CGI 인자들로 초기화한다. servedynamic:setenv
	setenv("REQUEST_METHOD", method, 1); //REQEUST_METHOD 환경 변수를 요청 URI의 CGI 인자들로 초기화한다. servedynamic:setenv
	Dup2(fd, STDOUT_FILENO); /* 자식은 자식의 표준 출력을 연결 파일 식별자로 재지정한다. Redirect stdout to client */ //servedynamic:dup2
	Execve(filename, emptylist, environ); /* CGI 프로그램을 로드하고 실행한다. Run CGI program */ //servedynamic:execve
    }
    Wait(NULL); /* Parent waits for and reaps child */ //servedynamic:wait
}
/* $end serve_dynamic */

