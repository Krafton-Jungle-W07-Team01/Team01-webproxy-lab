/*
 * adder.c - a minimal CGI program that adds two numbers together
 */
/* $begin adder */
#include "csapp.h"

int main(void) {
    char *buf, *buf2, *p, *a, *b;
    char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];
    int n1=0, n2=0;

    /* Extract the two arguments */
    if ((buf = getenv("QUERY_STRING")) != NULL) {
	p = strchr(buf, '&');           // buf에서 &를 찾는다
	*p = '\0';                                // &를 \0로 바꾸어 문자열을 끊는다
	strcpy(arg1, buf);             // arg1 에 & 앞 담고
	strcpy(arg2, p+1);            // arg2 에 & 뒤 담음

    a = strchr(arg1, '=');         // arg에서 = 을 찾는다
    b = strchr(arg2, '='); 
    if(a != NULL){                      // = 이 있다면
        *a = '\0';
        *b = '\0';
        strcpy(arg1, a+1);          // arg1과 2 에 value 를 각각 담는다.
        strcpy(arg2, b+1);
    }

	n1 = atoi(arg1);
	n2 = atoi(arg2);
    }

    if((buf2 = getenv("REQUEST_METHOD")) == NULL) {    // REQUEST_METHOD 환경변수가 null이라면, 즉 여기선 HEAD 를 넣지 않았다면.
        /* Make the response body */
        sprintf(content, "Welcome to add.com: ");
        sprintf(content, "%sTHE Internet addition portal.\r\n<p>", content);
        sprintf(content, "%sThe answer is: %d + %d = %d\r\n<p>", 
            content, n1, n2, n1 + n2);
        sprintf(content, "%sThanks for visiting!\r\n", content);
    }

    /* Generate the HTTP response */
    printf("Connection: close\r\n");
    printf("Content-length: %d\r\n", (int)strlen(content));
    printf("Content-type: text/html\r\n\r\n");
    printf("%s", content);
    fflush(stdout);

    exit(0);
}
/* $end adder */