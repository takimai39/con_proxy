#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <pthread.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <signal.h>
#include <netdb.h>

#define BUFSZ 1500

void copy(void * parm);
void print_hex(int len, unsigned char * bufptr);

struct sockaddr_in myaddr_in;
struct sockaddr_in peeraddr_in;
struct sockaddr_in serv_addr;

int reuseaddr = 1;

main(argc, argv)
int argc;
char *argv[];
{
    int addrlen, optval = 1;
    int ls , sds[2];
    struct hostent *hostp;
    pthread_t tid;

    if ( argc != 4 ) {
        printf ("%s: local-port remote-host remote-port \n", argv[0]);
        exit(1);
    }

    memset ((char *)&myaddr_in, 0, sizeof(struct sockaddr_in));
    memset ((char *)&peeraddr_in, 0, sizeof(struct sockaddr_in));
    memset ((char *)&serv_addr, 0, sizeof(struct sockaddr_in));

    /* Receive socket */
    myaddr_in.sin_family = AF_INET;
    myaddr_in.sin_addr.s_addr = INADDR_ANY;
    myaddr_in.sin_port = htons(atoi(argv[1]));

    if ( -1 == (ls = socket (AF_INET, SOCK_STREAM, 0)))
            perror("socket"), exit(1);

    /* Sender socket */
    if (!(hostp = gethostbyname(argv[2]))) {
        fprintf(stderr, "gethostbyname(): host infomation for \"%s\" not found\n", argv[2]);
        exit(1);
    }
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = ((struct in_addr*)(hostp->h_addr))->s_addr;
    serv_addr.sin_port = htons(atoi(argv[3]));

    setsockopt(ls, SOL_SOCKET,SO_REUSEADDR,&reuseaddr,sizeof(reuseaddr));

    if (bind(ls, (struct sockaddr *)&myaddr_in, sizeof(struct sockaddr)) == -1) 
    	perror("bind"), exit(1);


    if (listen(ls, 5) == -1)
    	perror("listen"), exit(1);

    while(1) {
        printf ("Waiting for contact ... \n");
        addrlen = sizeof(struct sockaddr_in);
        if ( -1 == (sds[0] = accept(ls, (struct sockaddr*)&peeraddr_in, &addrlen)))
            perror("accept"), exit(1);

        if ((sds[1] = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("socket");
            exit(1);
        }
        if (connect( sds[1], (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0 )
            perror("connect");

        if ( -1 == setsockopt( sds[1], IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval) ))
            perror("setsockopt");

        pthread_create( &tid, NULL, (void *(*)())copy, (void *)sds ) ;
        pthread_detach( tid ) ;
    }

}

void copy(void * parm)
{
    struct pollfd fds[2];
    int *sockfds;
    int state, len , mlen;
    int rec, snd;
    unsigned char buf[BUFSZ];
    struct sockaddr peeraddr_in;
    int addrlen;
    int opterr = 1;
    int optlen;

    sockfds = (int *) parm;
    rec = sockfds[0];
    snd = sockfds[1];

    if (fcntl(rec, F_SETFL, O_NONBLOCK) < 0) {
        perror("fcntl");
        exit(1);
    }
    if (fcntl(snd, F_SETFL, O_NONBLOCK) < 0) {
        perror("fcntl");
        exit(1);
    }

    fds[0].fd = rec;
    fds[0].events = POLLIN;

    fds[1].fd = snd;
    fds[1].events = POLLIN;

    while ( state = poll(fds, 2, -1 )) {
        if ( state == -1 ) {
            perror("poll"), close(snd), close(rec);
            pthread_exit(0);
        }

        if ( fds[0].revents & POLLIN ) {
            // printf("this is data for reading fd0\n");
            memset (buf, 0, BUFSZ );
            if ( -1 == (len = recv (rec , buf , BUFSZ, 0))) {
                perror("recv"), close(snd), close(rec);
                pthread_exit(0);
            }
            if ( len == 0 ) {	// disconnected 
                    close(snd), close(rec);
                    pthread_exit(0);
            }
            printf("                    VVVVV Receive VVVVV\n");
            print_hex(len, buf);
            while ( len > 0 ) {
                mlen = send (snd, buf, len, 0);
                len = len - mlen;
            }
        }

        if ( fds[1].revents & POLLIN ) {
            // printf("this is data for reading fd1\n");
            memset (buf, 0, BUFSZ );
            if ( -1 == (len = recv (snd , buf , BUFSZ, 0))) {
                perror("recv");
                close(snd), close(rec);
                pthread_exit(0);
            }
            if ( len == 0 ) {	// disconnected 
                    close(snd), close(rec);
                    pthread_exit(0);
            }
            printf("                    ^^^^^ Send ^^^^^\n");
            print_hex(len, buf);
            while ( len > 0 ) {
                mlen = send (rec, buf, len, 0);
                len = len - mlen;
            }
        }

        if ( fds[0].revents & POLLHUP || fds[1].revents & POLLHUP  || \
             fds[0].revents & POLLNVAL || fds[1].revents & POLLNVAL || \
             fds[0].revents & POLLERR || fds[1].revents & POLLERR )  {
            close(snd), close(rec);
            pthread_exit(0);
        }
    }
}

void print_hex(int len, unsigned char * bufptr )
{
    int i, j, k;
    char str[20];

    memset (str, 0, 20);

    for ( i = 0; i < len ; i++) {

        if ( i % 16 == 0 ) {
            printf("%.4x:", i );
        }

        if ( i % 4 == 0 )
            printf(" ");
  
        if ( bufptr[i] >= 0x20 && bufptr[i] <= 0x7e ) {
            str[i%16] = bufptr[i];
        }
        else {
            str[i%16] = '.';
        }

        printf ("%.2x ", bufptr[i]);

        if (i%16 == 15 ){
            printf(" | %s\n", str );
            memset (str, 0, 20 );
        }
    }
   
    j = len % 16;
    if ( j > 0 ) {
        for ( k = 0 ; k < (16-j); k++ ) {
            if ( (j + k)%4 == 0 )
                printf(" ");
            printf ("   ");
        }
        printf(" | %s\n", str );
    }
}
