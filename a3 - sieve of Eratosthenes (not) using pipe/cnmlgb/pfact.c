#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


int belones_to_m(int cl, int m[255]){
	int m_index =0;
	while (m[m_index] != -1){
		if (cl == m[m_index]){
			return 1;
		}
		m_index++;
	}
	if (m[m_index - 1] < cl){
		return 1;
	}
	else{
		return 0;
	}
}



int main(int argc, char *argv[]){
	int n;
	n = atoi(argv[1]);
	
	int i;
	int fd[2];
	int status;
	int factor;
	int m[255];
	int m_index = 0;
	int cl = 2;
	int sig_type;
	int stage_number;
	
	double n_sqrt;
	n_sqrt = sqrt((double) n);
	int m_sqrt = (int) n_sqrt;
	
	pid_t pid_m, pid;
	//initialize list of m;
	for (m_index = 0; m_index <= 255; m_index++){
		m[m_index] = -1;
	}
	m_index = 0;
	
	i = 2;
	//construct pipe
	if (pipe(fd) == -1){
		perror("pipe");
		exit(1);
	}
	while (i <= n){
		
		if ((pid_m = fork()) == -1){
			perror("fork");
			exit(1);
		}
		
		//first child
		if(pid_m == 0){
			int init_m = 0;
			/*
			if((r = read(fd[0], &ariango, sizeof(ariango))) == -1) {
				perror("read");
				exit(1);
			} else if(r == 0) {
				fprintf(stderr, "Error: could not read from stdin");
				exit(1);
			} */
			
			while (cl <= i){
				//printf("current pid = %d    init_m = %d\n", pid, init_m);
				printf("i = %d    n = %d     factor = %d\n" , i, n, factor);
				printf("cl = %d    init_m = %d\n", cl, init_m);
				printf("current m = [%d, %d, %d, %d]    \n", m[0], m[1], m[2], m[3]);
				//case: cl arrived at i
				if(cl == i){
					m[m_index] = i;
					//i is a factor of n
					if ((n % i) == 0){
						if (factor == 0){
							//n is the square of i
							if ((i * i) == n){
								exit((1 << 3) + 1);
							}
							//n itself is a prime
							else if(i == n){
								exit((1 << 3) + 6);
							}
							//n is i * another number
							else{
								exit((1 << 3) + 2);
							}
						}
						// i is another factor two factor's product is not n
						else if ((i * factor) != n){
							exit((1 << 3) + 7);
						}
					}
					//i is not a facor of n
					else{
						exit((1 << 3) + 3);
					}
				}
				//case cl havn't arrived at i
				else {
					
					//check if i is decarded
					if(((i % m[init_m]) == 0) & (m[init_m] != -1)){
						exit((1 << 3) + 4);
					}
					//cl arrived a new m
					else if(belones_to_m(cl, m)){
						
						//i is another factor with this stage
						if((i * m[init_m]) == n){
							exit((1 << 3) + 5);
						}
						//i need to be pushed to next stage
						else{
							
							if((pid = fork()) == -1){
								perror("fork");
								exit(1);
							}
							//child: m become next stage's m
							if (pid == 0){
								init_m++;
								cl++;
							}
							//father: wait child's signal
							else{
								wait(&status);
								exit(WEXITSTATUS(status) + (1 << 3));
							}
						}
					}
					else {
						cl++;
					}
				}
				
			}
			
		}
		/* Child signal rule:
		 * a binary number in "...aaabbb" form.
		 * "...aaa" part represents the stage number
		 * "bbb" represents exit situation:
		 * 1 = ( n is square i)
		 * 2 = ( first factor = i)
		 * 3 = ( i is not a factor of n but new stage constructed)
		 * 4 = ( i is decarded)
		 * 5 = ( i is another factor and done)
		 * 6 = ( i itself is a prime)
		 * 7 = ( i has more than two factor)
		 */
		//master
		else{
			
			if(write(fd[1], &m_sqrt, sizeof(m_sqrt)) == -1) {
				perror("write to pipe");
			}
			
			wait(&status);
			
			sig_type = WEXITSTATUS(status) & 7;
			stage_number = WEXITSTATUS(status) >> 3;
			printf("situation type ----> %d\n", sig_type);
			if ((double)i > n_sqrt){
				if (factor == 0){
					printf("%d is prime\n", n);
					printf("Number of stages = %d\n", stage_number);
					exit(0);
					
				}
			}
			
			//situation 1 = ( n is square i)
			if (sig_type == 1){
				//exit status is not 1
				if (WEXITSTATUS(status) != 1){
					printf("%d %d %d\n", n, i, i);
					printf("Number of stages = %d\n", stage_number);
					close(fd[0]);
					close(fd[1]);
					exit(0);
				}
				//exit status is 1 -> this is an error exit
				else{
					perror("Even I don't understand but this is an error");
					exit(1);
				}
			}
			//situation 2 = ( first factor = i)
			else if (sig_type == 2){
				m[m_index] = i;
				m_index++;
				factor = i;
				i++;
			}
			//situation 3 = ( i is not a factor of n but new stage constructed)
			else if (sig_type == 3){
				m[m_index] = i;
				m_index++;
				i++;
			}
			//situation 4 = ( i is decarded)
			else if (sig_type == 4){
				i++;
			}
			//situation 5 = ( i is another factor and done)
			else if (sig_type == 5){
				printf("%d %d %d\n", n, factor, i);
				printf("Number of stages = %d\n", stage_number);
				close(fd[0]);
				close(fd[1]);
				exit(0);
			}
			//situation 6 = ( i itself is a prime)
			else if (sig_type == 6){
				printf("%d is prime\n", n);
				printf("Number of stages = %d\n", stage_number);
				close(fd[0]);
				close(fd[1]);
				exit(0);
			}
			//situation 7 = ( i has more than two factor)
			else if (sig_type == 7){
				printf("%d is not the product of two primes\n", n);
				printf("Number of stages = %d\n", stage_number);
				close(fd[0]);
				close(fd[1]);
				exit(0);
			}
		}
		//printf("factor = %d\n", factor);
	} 
	//all number have been checked, but havn't get another factor
	//end with situation 2 = ( first factor = i) -> it have another factor greater than n -> impossible
	if(sig_type == 2){
		perror("factor larger than n");
		exit(1);
	}
	//end with situation 3 = ( i is not a factor of n but new stage constructed) -> n it self is a prime
	else if(sig_type == 3){
		printf("%d is prime\n", n);
		printf("Number of stages = %d\n", stage_number);
		close(fd[0]);
		close(fd[1]);
		exit(0);
	}
	//end with //situation 4 = ( i is decarded)
	else if(sig_type == 4){
		if (factor == 0) {
			printf("%d is prime\n", n);
			printf("Number of stages = %d\n", stage_number);
			close(fd[0]);
			close(fd[1]);
			exit(0);
		}
		else{
			printf("%d is not the product of two primes\n", n);
			printf("Number of stages = %d\n", stage_number);
			close(fd[0]);
			close(fd[1]);
			exit(0);
		}
	}
	
	return 0;
}
	
	 
