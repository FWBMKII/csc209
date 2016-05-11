#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <pthread.h>

#include "crc16.h"
#include "xmodemserver.h"
/*
#ifndef port
  #define port 50526
#endif
*/
int port = 50526;

struct client *top = NULL;
int howmany = 1;  

//partly cited from muffinman.c
//construct the 
static void addclient(struct client *p,int fd, int state)
{
    if (!p) {
	fprintf(stderr, "out of memory!\n");  /* highly unlikely to happen */
	exit(1);
    }
    printf("Adding client\n");
    fflush(stdout);
	p->state = state;
    p->fd = fd;
    p->next = top;
    top = p;
}

//partly cited from muffinman.c
static void removeclient(int fd){
    struct client **p;
    for (p = &top; *p && (*p)->fd != fd; p = &(*p)->next)
		;
    if (*p) {
		struct client *t = (*p)->next;
		fflush(stdout);
		free(*p);
		*p = t;
		howmany--;
    } 
    else {
		fprintf(stderr, "Trying to remove fd %d, but I don't know about it\n", fd);
		fflush(stderr);
    }
}

int main(int argc, char **argv) {
	int c, fd;
    int listenfd;
    fd_set	rset;
    struct sockaddr_in servaddr;
    struct client *p;
    char *dir = "filestore";
    char temp;
    FILE *fp;
    
    unsigned char block;
    unsigned char inverse;
    unsigned short crc;
    unsigned char crc1;
    unsigned char crc2;
    
    
    /*
    while ((c = getopt(argc, argv, "p:")) != EOF) {
		if (c == 'p') {
			if ((port = atoi(optarg)) == 0) {
			fprintf(stderr, "%s: non-numeric port \"number\"\n", argv[0]);
			return(1);
			}
		} else {
			fprintf(stderr, "usage: %s [-p port]\n", argv[0]);
			return(1);
		}
    }
    */
    //___________________________
    //construct socket
	printf("constructing socket...\n");
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		exit(1);
    }

    memset(&servaddr, '\0', sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port        = htons(port);

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof servaddr)) {
		perror("bind");
		exit(1);
    }

    if (listen(listenfd, 10)) {
		perror("listen");
		exit(1);
    }
    printf("socket construction complete\n");
    
    //------------------------------------------
    while (1){
		fd_set fdlist;
		int maxfd = listenfd;
		FD_ZERO(&fdlist);
		FD_SET(listenfd, &fdlist);
		printf("adding fdlist...\n");
		for (p = top; p; p = p->next) {
			FD_SET(p->fd, &fdlist);
			if (p->fd > maxfd)
				maxfd = p->fd;
		}
		printf("adding complete\n");
		printf("!  first select, waiting for signal\n");
		if (select(maxfd + 1, &fdlist, NULL, NULL, NULL) < 0) {
			perror("select");
		} else {
			printf("start checking p after select...\n");
			for (p = top; p; p = p->next)
				if (FD_ISSET(p->fd, &fdlist))
					break;
			printf("cheking for p complete\n");
			if (FD_ISSET(listenfd, &fdlist)){
				struct sockaddr_in r;
				socklen_t socklen = sizeof r;
				if ((fd = accept(listenfd, (struct sockaddr *)&r, &socklen)) < 0) {
					perror("accept");
				}
				if (p == NULL){
					p = malloc(sizeof(struct client));
				}
				
				//socket accepted
				
				addclient(p, fd, initial);
				printf("addclient(fd) complete\n");
				printf("p->state = %d, %d\n", p->state, initial);
				//___________________
				//
				while (p->state != finished){
					if (p->state == initial){
						//~~~~~~~~Current state: initial~~~~~~~~~
						FD_ZERO(&rset);
						FD_SET(fd, &rset);
						if (select(fd + 1, &rset, NULL, NULL, NULL) < 0) {
							perror("select");
						} else {
							//reading filename...
							char nmb;
							c = 0;
							while (c < 20){
								read(fd, &nmb, 1);
								p->filename[c] = nmb;
								c++;
								if (nmb == '\n'){
									break;
								}
							}
							//read(fd, p->filename, 20);
							printf("filename is %s\n", p->filename);
							if((fp = open_file_in_dir(p->filename, dir)) == NULL) {
								perror("open file");
								exit(1);
							}else{
								p->fp = fp;
								//sending 'C' back
								temp = 'C';
								write(fd, &temp, 1);
								
								p->current_block = 1;
								p->state = pre_block;
							}
						}
					}
					if (p->state == pre_block){
						//~~~~~~~~~~~~current stage: pre_block~~~~~~~~~
						//get ASCII code
						
						FD_ZERO(&rset);
						FD_SET(p->fd, &rset);
						//start select
						if (select(p->fd + 1, &rset, NULL, NULL, NULL) < 0) {
							perror("select");
							continue;
						}else{
							//start reading temp...
							if((read(fd, &temp, 1)) == -1){
								perror("read");
							}
							printf("temp is %d\n", temp);
							if(temp == EOT){
								//case: EOT\n
								temp = ACK;
								write(fd, &temp, 1);
								p->state = finished;
							}
							else if(temp == SOH){
								//case: SOH
								p->state = get_block;
								p->blocksize = 128;
							}
							else if(temp == STX){
								//case: STX
								p->state = get_block;
								p->blocksize = 1024;
							}
						}
					}
					if (p->state == get_block){
						//~~~~~~~~current stage: get_block~~~~~~~~~~
						if(p->blocksize == 128){
							//case: SOH
							int i;
							for (i = 0; i < 132; i++){
								read(fd, &temp, 1);
								p->buf[i] = temp;
								p->inbuf = i;
							}
							p->buf[132] = '\0';
							p->state = check_block;
							
						}
						else if(p->blocksize == 1024){
							//case: STX
							int i = 0;
							for (i = 0; i < 1028; i++){
								read(fd, &temp, 1);
								p->buf[i] = temp;
								p->inbuf = i;
							}
							p->buf[1028] = '\0';
							p->state = check_block;
						}
					}
					if (p->state == check_block){
						//~~~~~~~~current stage: check_block~~~~~~~~~~
						block = p->buf[0];
						inverse = p->buf[1];
						printf("block = %d; inverse = %d\n", block, inverse);
						
						if((255 - block) != inverse){
							//case: block + 255 != inverse
							p->state = finished;
						}
						else if(block == (p->current_block - 1)){
							//case: repeat block
							temp = ACK;
							write(fd, &temp, 1);
							
						}
						else if(block != (p->current_block)){
							p->state = finished;
						}
						else{
							//case: check crc
							unsigned char buf[p->blocksize];
							int i = 0;
							for (i = 0; i < p->blocksize; i++){
								buf[i] = p->buf[i+2];
							}
							
							crc = crc_message(XMODEM_KEY, buf, p->blocksize);
							printf("crc i culculate is: %x\n", crc);
							printf("crc should be: %x, %x\n", 0xff&p->buf[p->inbuf - 1], 0xff&p->buf[p->inbuf]);
							crc1 = (crc >> 8) & 0xff;
							crc2 = crc & 0xff;
							printf("crc i culculate is (2 parts): %x, %x\n", crc1, crc2);
							printf("crc1 - crc2 = %x\n", crc1 - (0xff&p->buf[p->inbuf - 1]));
							if(((crc1) != (0xff&p->buf[p->inbuf - 1])) || ((crc2) != (0xff&p->buf[p->inbuf]))){
								temp = NAK;
								write(fd, &temp, 1);
							}
							else{
								//case: good block!!!! 
								p->current_block++;
								printf("%s\n", buf);
								fwrite(buf, p->blocksize, 1, fp);
								temp = ACK;
								write(fd, &temp, 1);
								p->state = pre_block;
								
							}
						}
						
					}
				}
				if (p->state == finished){
					//~~~~~~~~current stage: finished~~~~~~~~~~`
					
					//fd = p->fd;
					removeclient(fd);
					close(fd);
				}
			}
		}
		
	}
    return 0;
}



