/* $begin hostinfo */
#include "csapp.h"

int main(int argc, char **argv)
{
    struct addrinfo *p, *listp, hints;
    char buf[MAXLINE];
    int rc, flags;

    if (argc != 2 && argc != 3) { // (argv[1] : 호스트 네임, argv[2] : 서비스 네임)
	fprintf(stderr, "usage: %s <domain name>\n", argv[0]);
	exit(0);
    }

    /* Get a list of addrinfo records */
    memset(&hints, 0, sizeof(struct addrinfo)); // hints 구조체 초기화
    hints.ai_family = AF_INET;       /* IPv4 only */        //line:netp:hostinfo:family
    hints.ai_socktype = SOCK_STREAM; /* Connections only */ //line:netp:hostinfo:socktype
    rc = getaddrinfo(argv[1], argv[2], &hints, &listp); // 도메인 네임 리턴
    if (rc != 0) { // 오류 발생
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(rc));
        exit(1);
    }

    /* Walk the list and display each IP address */
    flags = NI_NUMERICHOST; /* 10진수 숫자로 포팅된 형태로 리턴받는다. Display address string instead of domain name */
    for (p = listp; p; p = p->ai_next) {
        getnameinfo(p->ai_addr, p->ai_addrlen, buf, MAXLINE, NULL, 0, flags);
        printf("%s\n", buf);
    }

    /* Clean up */
    Freeaddrinfo(listp);

    exit(0);
}
/* $end hostinfo */
