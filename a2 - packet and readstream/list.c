#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "data.h"


char           *messages[] = {
    "inserted packet",
    "CRC code matches",
    "CRC code does not match",
    "duplicate block"
};

void log_message(struct packet *p, int message_no, FILE *logfp) {
    fprintf(logfp, "%hu\t%hu\t%s", p->block_num, p->block_size, 
            messages[message_no]);
}
