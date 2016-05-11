#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>

#include "data.h"

FILE           *logfp = NULL;


/* Read a packet file, and coalesce packets */

/*
 * Print to stdout the reconstructed data.  If a packet is missing, insert
 * ### into the output
 */

int main(int argc, char *argv[]) {
    FILE *infp;
    char opt;
    extern int optind;
    extern char *optarg;

    while ((opt = getopt(argc, argv, "l:")) != -1) {
        switch (opt) {
            case 'l':
                logfp = fopen(optarg, "w");
                if(!logfp) {
                    perror("fopen");
                    exit(1);
                }
                break;
            default: /* '?' */
                fprintf(stderr, "Usage: %s [-l logfile ] inputfile\n",
                        argv[0]);
                exit(1);
        }
    }
    if(optind >= argc) {
        fprintf(stderr, "Expected inputfile name\n");
        exit(1);
    }

    if(argc != 2 && argc != 4) {
        fprintf(stderr, "Usage: %s [-l logfile ] inputfile\n", argv[0]);
        exit(1);    
    }

    if((infp = fopen(argv[optind], "r")) == NULL) {
        perror("fopen");
        exit(1);
    }
    /*----------------------------------------------------------------*/
    
    //construct the linked list.
    
    struct list *curr;
    struct list *l;
    l = create_node(infp);
    curr = l;
    while (!feof(infp)){
		curr->next = create_node(infp);
		curr = curr->next;
	}
	

	
	
	//sort the linked list.
	sort_node(l);
	curr = l;
	struct list *j;
	while (curr->next != NULL){
		if((curr->next->p.block_num == 0) 
		& (curr->next->p.block_size == 0) & (curr->next->p.crc == 0)){
			j = curr->next->next;
			free(curr->next->p.payload);
			if(curr->next->p.payload == NULL) {
				perror("free");
				exit(1);
			}
			free(curr->next);
			if(curr->next == NULL) {
				perror("free");
				exit(1);
			}
			curr->next = j;
		}
		else{
			curr = curr->next;
		}
	}
	
	curr = l;
	while (curr->next != NULL){
		if (logfp != NULL){
			log_message(&curr->p, 0, logfp);
			unsigned short crc = crc_message(XMODEM_KEY, curr->p.payload, curr->p.block_size);
			if(crc != curr->p.crc){
				log_message(&curr->p, 2, logfp);
			}
			else{
				log_message(&curr->p, 1, logfp);
			}
		}
		curr = curr->next;
	}
	
	//close the file.
	fclose(infp);
	fclose(logfp);
	//free the malloced memory.
	
	curr = l;
	l = NULL;
	free(l);
	while (curr != NULL){
		j = curr->next;
		free(curr->p.payload);
		
		if(curr->p.payload == NULL) {
			perror("free");
			exit(1);
		}
		free(curr);
		
		if(curr == NULL) {
			perror("free");
			exit(1);
		}
		curr = j;
	}

    return 0;
    
}

//create a node with information read from the file.
struct list *create_node(FILE *infp){
	struct list *i;
	i = malloc(sizeof(struct list));	
	if(i == NULL) {
		perror("malloc error");
		exit(1);
	}
	fread(&i->p, sizeof(struct packet), 1, infp);
	i->p.payload = malloc(i->p.block_size);
	if(i->p.payload == NULL) {
		perror("malloc error");
		exit(1);
	}
	fread(i->p.payload, 1, i->p.block_size, infp);

	i->next = NULL;
	return i;
}

//bubble sort the linked list.
int sort_node(struct list *l){
	struct list *curr;
	
	int loop_ctrl = 0;
	while (loop_ctrl == 0){
		curr = l;
		loop_ctrl = 1;
		while (curr->next->next != NULL){
			
			if(curr->p.block_num >= curr->next->p.block_num){
				exchange_node(curr, curr->next);
				loop_ctrl = 0;
			}
			curr = curr->next;
		}
	}
	return 0;
}

//Exchange the position of i and j two nodes, remove the old node if two
//nodes have the same block_num. 
int exchange_node(struct list *i, struct list *j){
	if (i->p.block_num != j->p.block_num){		
		struct packet cur = i->p;
		i->p = j->p;
		j->p = cur;
	}
	else {
		if (logfp != NULL){
			log_message(&i->p, 3, logfp);
		}
		unsigned char *cnmb = i->p.payload;
		i->p.block_size = j->p.block_size;
		i->p.crc = j->p.crc;
		i->p.payload = j->p.payload;
		i->next = j->next;
		free(cnmb);
		free(j);
		if(j == NULL) {
			perror("free");
			exit(1);
		}
	}
	return 0;
}
