#include "csapp.h"

int main(int argc, char **argv)
{
	struct addrinfo	*p, *listp, hints;
	char			buf[MAXLINE];
	int 			rc, flags;

	if (argc != 2)
	{
		fprintf(stderr, "usage: %s <domain name>\n", argv[0]);
		exit(0);
	}

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if ((rc = getaddrinfo(argv[1], NULL, &hints, &listp) != 0))
	{
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(rc));
		exit(1);
	}

	flags = NI_NUMERICHOST;
	for (p = listp; p; p = p -> ai_next)
	{
		getnameinfo(p -> ai_addr, p -> ai_addrlen, buf, MAXLINE, NULL, 0, flags);
		printf("%s\n", buf);
	}

	freeaddrinfo(listp);

	exit(0);
}
