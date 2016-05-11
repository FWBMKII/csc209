#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

void my_handler(int code) {
    if (code == 2) {
       printf("I got a 2\n");
    }
    fprintf(stderr, "\nI don't want to quit!\n");
    sleep(5);
    fprintf(stderr, "I received code %d\n",code);
    exit(1); 
}

int main() {

     struct sigaction newact;
     newact.sa_handler = my_handler;
     newact.sa_flags = 0;

     /* let's set up a set of signals to block while we
     handle the SIGINT */

     sigset_t my_delay_set;
     sigemptyset(&my_delay_set);
     if (sigaddset(&my_delay_set, SIGTERM) != 0) {
        perror("sigaddset");
        exit(1);
     }
     
     newact.sa_mask = my_delay_set;

     sleep(5);
     if (sigaction(SIGINT, &newact, NULL) != 0) {
         perror("sigaction");
         exit(1);
     }

     int i = 0;

	/* compute for a while */
	for(;;) 
	    if ((i++ % 50000000) == 0)	 fprintf(stderr,".");
}
