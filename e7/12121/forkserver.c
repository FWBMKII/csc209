/*
** server.c -- a stream socket server demo
*/

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
#include <signal.h>

#ifndef PORT
#define PORT 11491    /* the port users will be connecting to */
#endif
#define MAXDATASIZE 100 /* max number of bytes we can get at once  */

#define BACKLOG 10     /* how many pending connections queue will hold*/

ssize_t Readn(int, void *, size_t);
ssize_t Readline(int, void *, size_t);
void Writen(int, void *, size_t);
void sigchld_handler(int s)
{
  wait(NULL);
}

int main(void)
{
  int sockfd, new_fd;  /* listen on sock_fd, new connection on new_fd*/
  struct sockaddr_in my_addr;    /* my address information */
  struct sockaddr_in their_addr; /* connector's address information */
  unsigned int sin_size;
  struct sigaction sa;
  char buf[MAXDATASIZE];

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    exit(1);
  }

  my_addr.sin_family = AF_INET;         /* host byte order */
  my_addr.sin_port = htons(PORT);     /* short, network byte order */
  my_addr.sin_addr.s_addr = INADDR_ANY; /* automatically fill with my IP */
  bzero(&(my_addr.sin_zero), 8);        /* zero the rest of the struct */

  if (bind(sockfd, (struct sockaddr *)&my_addr, 
           sizeof(struct sockaddr)) == -1) {
    perror("bind");
    exit(1);
  }

  if (listen(sockfd, BACKLOG) == -1) {
    perror("listen");
    exit(1);
  }

  sa.sa_handler = sigchld_handler; /* reap all dead processes */
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  while(1) {  /* main accept() loop*/
    sin_size = sizeof(struct sockaddr_in);
	printf("Calling accept...\n");
    if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, 
                         &sin_size)) == -1) { 
      perror("accept");
      continue;
    }
    printf("completed accept\n");
    printf("server: got connection from %s\n",
	       inet_ntoa(their_addr.sin_addr));

    if (!fork()) { /* this is the child process */
      printf("(%d) I'm the child: I'll be writing to the client\n", new_fd);
      close(sockfd); /* child doesn't need the listener */

      Writen(new_fd, "Who are you?\n", 14);
      while(( Readline(new_fd, buf, MAXDATASIZE)) != 0) {
      	printf("(%d) I received %s\n", new_fd, buf);
      }

      close(new_fd);
      printf("Child is exiting\n");
      exit(0);
    }
	/* only parent gets here */
    //    close(new_fd);  /* parent doesn't need this */
  }

  return 0;
}
