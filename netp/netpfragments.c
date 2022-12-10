#include "csapp.h"

void fragments()
{
    int clientfd;

    /* $begin socketcall */
    /*
        소켓 식별자를 생성
        소켓을 끝점으로 만들고 싶으면 하드코딩으로 socket 함수를 호출할 수 있다.
        AF_INET : 32비트 IP주소를 사용하고 있음을 나타냄
        SOCK_STERAM : 소켓이 인터넷 연결의 끝점이 될 것을 나타냄

        다른 방식 : 이러한 매개변수를 자동으로 생성해서 코드가 프로토콜에 
        무관하게 되도록 getaddrinfo 함수를 사용하는 게 더 좋다.
    */
    clientfd = Socket(AF_INET, SOCK_STREAM, 0);
    /* $end socketcall */

    clientfd = clientfd; /* keep gcc happy */

    /* $begin inaddr */
    /* IP address structure, 32비트 IP 주소를 저장할 구조체 */
    struct in_addr {
        uint32_t  s_addr; /* Address in network byte order (big-endian) */
    };
    /* $end inaddr */

    /* $begin addrinfo */
    /**
     * @brief getaddrinfo가 리턴하는 소켓 주소들의 구조체 
     * 
     */
    struct addrinfo {
        int              ai_flags;     /* 추가적인 옵션 정의, Hints argument flags */
        int              ai_family;    /* 주소 체계(AF_INET, AF_INET6, AF_UNSPEC) First arg to socket function */
        int              ai_socktype;  /* socket type(SOCK_SREAM, SOCK_DGRAM), Second arg to socket function */
        int              ai_protocol;  /* 프로토콜을 식별하기 위한 매개변수 값(IPPROTO_XXX), Third arg to socket function  */
        char            *ai_canonname; /* 별칭 호스트 네임 -> 내부적으로 다시 질의, CNAME(Canonical hostname) */
        size_t           ai_addrlen;   /* ai_addr의 길이, Size of ai_addr struct */
        struct sockaddr *ai_addr;      /* socket 주소를 나타내는 구조체 포인터, Ptr to socket address structure */
        struct addrinfo *ai_next;      /* 다음 데이터의 포인터(linked list 이므로) Ptr to next item in linked list */
    };
    /* $end addrinfo */


    /* $begin sockaddr */
    /* IP socket address structure */
    /*
        IPv4 주소체계에서 사용하는 구조체
        소켓 프로그램은 범용 주소 구조체로 sockaddr을 사용하지만 주소체계의 종류에 따라
        별도의 전용 구조체를 만든 것이다.

        소켓 라이브러리는 sockaddr를 사용하므로 라이브러리에 주소 정보를 넘길 때는
        sockaddr로 형변환을 해서 넘긴다.


        sockaddr 구조체에서 sa_family가 AF_INET인 경우에 사용하는 구조체이다.
        sockaddr을 그대로 사용할 경우, sa_data에 IP주소와 Port번호가 조합되어 있어 쓰거나 읽기 불편하다.
    */
    struct sockaddr_in  {
        uint16_t        sin_family;  /* 주소 체계, Protocol family (always AF_INET) */
        uint16_t        sin_port;    /* 16 비트 포트 번호, Port number in network byte order */
        struct in_addr  sin_addr;    /* 32비트 IP 주소, IP address in network byte order */
        unsigned char   sin_zero[8]; /* 전체 크기를 16비트로 맞추기위한 패딩, Pad to sizeof(struct sockaddr) */
    };

    /* Generic socket address structure (for connect, bind, and accept)
        소켓의 틀(크기)를 잡는다.
    */
    struct sockaddr {
        uint16_t  sa_family;    /* 주소 체계, Protocol family */
        char      sa_data[14];  /* 해당 주소 체계에서 사용하는 주소 정보, Address data  */
    };	
    /* $end sockaddr */

}
