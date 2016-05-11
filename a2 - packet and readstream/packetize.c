#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>

#include "data.h"

FILE           *logfp = NULL;


/* Read a file, break it into packets. */

/*
 * Notes: getopt is a useful library function to make it easier to read in
 * command line arguments, especially those with options. Read the man page
 * (man 3 getopt) for more information.
 */

int main(int argc, char *argv[]) {
    FILE *infp = stdin;
    FILE *outfp = NULL;
    char opt;
    extern int optind;
    extern char *optarg;

    while ((opt = getopt(argc, argv, "f:")) != -1) {
        switch (opt) {
            case 'f':
                infp = fopen(optarg, "r");
                if(!infp) {
                    perror("fopen");
                    exit(1);
                }
                break;
            default: /* '?' */
                fprintf(stderr, "Usage: %s [-f inputfile ] outputfile\n",
                        argv[0]);
                exit(1);
            }
    }

    if(optind >= argc) {
        fprintf(stderr, "Expected outputfile name\n");
        exit(1);
    }

    if(!(outfp = fopen(argv[optind], "w"))){
        perror("fopen");
        exit(1);
    }

    /* The files have been opened for you.  Write the rest of the program here.*/

    /*
    struct packet *p = malloc(sizeof(struct packet));
    int num_index = 0;
    while (!feof(infp)){
        
    }
    free(p);
    fclose(infp);
    fclose(outfp);
    */
    
    
    struct list *curr;
    struct list *l;
    int bn = 0;
    l = pcreate_node(infp, outfp, bn);
    l->p.block_num = bn;
    bn +=1;
    printf("num: %d, size: %d, crc: %x, payload:%s\n",l->p.block_num, l->p.block_size, l->p.crc, l->p.payload);
    curr = l;
    while (!feof(infp)){
		curr->next = pcreate_node(infp, outfp, bn);
	    printf("num: %d, size: %d, crc: %x, payload:%s\n",curr->p.block_num, curr->p.block_size, curr->p.crc, curr->p.payload);
		bn +=1;
		curr = curr->next;
	}
    
    	
	//close the file.
	fclose(infp);
	fclose(outfp);
	//free the malloced memory.
	
	curr = l;
	l = NULL;
	struct list *j;
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

//create a node and store it with a packet.
struct list *pcreate_node(FILE *infp, FILE *outfp, int bn){
	struct list *i;
	i = malloc(sizeof(struct list));
	if(i == NULL) {
		perror("malloc error");
		exit(1);
	}
	i->p.payload = malloc(MAXSIZE);
	if(i->p.payload == NULL) {
		perror("malloc error");
		exit(1);
	}
	i->p.block_num = bn;
    i->p.block_size = fread(i->p.payload, sizeof(char), MAXSIZE, infp);
    i->p.crc = crc_message(XMODEM_KEY, i->p.payload, i->p.block_size);
    fwrite(&i->p, sizeof(struct packet), 1, outfp);
    fwrite((i->p).payload, 1, MAXSIZE, outfp);
	return i;
}


