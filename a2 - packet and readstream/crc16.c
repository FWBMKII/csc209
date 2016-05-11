#include <string.h>
#include <stdio.h>

#include "data.h"

// based on code from:
// http://stackoverflow.com/questions/111928/is-there-a-printf-converter-to-print-in-binary-format?rq=1

const char *to_binary(int x) {
    static char bits[17];
    bits[0] = '\0';
    int z;
    for (z = 1 << 15; z > 0; z >>= 1) {
        strcat(bits, ((x & z) == z) ? "1" : "0");
    }
    return bits;
}


/*  
  Process one bit of the message.
  reg: register (modified by this function)
  key: CRC16 key being used
  next_bit: next bit of message
*/

void crc_bit(unsigned short *reg, unsigned int key, unsigned int next_bit) {
    unsigned int sig;
    sig = (*reg & (1 << 15)) >> 15;
    if (sig != 1){
        *reg = ((*reg << 1)) | (next_bit);
    }
    else {
        *reg = ((*reg << 1)) | (next_bit);
        *reg ^= key;
    }
    //printf("%d\n", (next_bit));
    //printf("%s\n", (to_binary(*reg)));
}
  
/*
  Process one byte of the message.
  reg: register (modified by this function)
  key: CRC16 key being used
  next_byte: next byte of message
*/

void crc_byte(unsigned short *reg, unsigned int key, unsigned int next_byte) {
    unsigned int next;
    int i;
    for (i = 0; i <= 7; i++){
        next = (1 << (7 - i));
        //printf("%d\n", (233));
        crc_bit(reg, key, ((next_byte & next) >> (7-i)));
    }
}

/*
  Process an entire message using CRC16 and return the CRC.
  key: CRC16 key being used
  message: message for which we are calculating the CRC.
  num_bytes: size of the message in bytes.
*/

unsigned short crc_message(unsigned int key, unsigned char *message, int num_bytes) {
	unsigned short reg_address = 0x0000;
    unsigned short *reg = &(reg_address);
    int i;
    int byte_index;
    
    i = num_bytes; 
    //printf("%s\n", (to_binary(key)));
    for (; i > 0; i--) {
		byte_index = num_bytes - i;
		crc_byte(reg, key, (message[byte_index]));
    }
    for (i = 0; i < 2; i++){
		crc_byte(reg, key, 0x0000);
	}
    return *reg;
}


/*testing code*/
/*
int main(void) {
  //unsigned short b = 0xffff;
  //unsigned short *a = &(b); 
  //printf("%d\n", ((*a & (1 << 15)) >> 15));
  unsigned short crc16 = crc_message(XMODEM_KEY, "d", 1);
  printf("%s\n", to_binary(crc16));
  return 23333;
}
*/
