#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

int numkids = 0;

/* Step 1: Write a signal handling function */
void lzsb(int code){
	
	int status;
	int exit_status;
	wait(&status);
	
	exit_status = WEXITSTATUS(status);
	numkids--;
	printf("child exited with status %d\n", exit_status);
	
}



/* don't change the child code */
void do_child(int cnum) {
    printf("Child %d\n", cnum);
    sleep(2*cnum);
    exit(cnum);
}

int main(int argc, char **argv) {
    int i;
    pid_t pid;
    struct sigaction sa;

    if(argc != 2) {
        fprintf(stderr, "Usage: sigchld <num children>\n");
        exit(1);
    }
    numkids = atoi(argv[1]);

  /* Turn off buffering */
    setbuf(stdout, NULL);

  /* Step 2: Install signal handler */
	sa.sa_handler = lzsb;
	sa.sa_flags = 0;

	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		exit(1);
	}

  /* create some children */
  /* Don't change any code below this line */
  /* NOTE: This code was fixed to use k rather than numkids
   * as the stopping condition because numkids could change.*/
    int k = numkids;
    for (i = 0; i < k; i++) {
        if ((pid = fork()) == -1) {
            perror("fork"); exit(1);
            
        } else if (pid == 0) {
        /* child */
            printf("child %d created\n", i);
            do_child(i);
        }
    }

  /* compute for a while */
    i = 0;
    while(numkids > 0) {
        i++;
        if (i % 10000000 == 0)
            putchar('.');
    }
    printf("\ndone computing\n");

    return 0;
}

