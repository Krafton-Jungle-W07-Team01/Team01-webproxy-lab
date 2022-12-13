/*
 * echoclient.c - An echo client
 */
/* $begin echoclientmain */
#include "csapp.h"

int main(int argc, char **argv) 
{
    int clientfd;
    char *host, *port, buf[MAXLINE];
    rio_t rio;

    if (argc != 3) {
	fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
	exit(0);
    }
    host = argv[1];
    port = argv[2];

    clientfd = Open_clientfd(host, port); // 서버와의 연결을 수립한다.
    Rio_readinitb(&rio, clientfd);

    while (Fgets(buf, MAXLINE, stdin) != NULL) {
	Rio_writen(clientfd, buf, strlen(buf)); // 서버에 텍스트 줄을 전송한다.
	Rio_readlineb(&rio, buf, MAXLINE); // 서버에서 echo줄을 읽는다.
	Fputs(buf, stdout); // 해당 결과를 표준 출력으로 인쇄한다.
    }
    // 클라이언트 커널이 프로세스를 종료할 때 자동으로 열었던 모든 식별자들을 닫아주기에
    // close는 불필요하다. But, 열었던 모든 식별자들을 명시적으로 닫아주는게 
    // 올바른 프로그래밍 습관이다.
    Close(clientfd); //line:netp:echoclient:close
    exit(0);
}
/* $end echoclientmain */
