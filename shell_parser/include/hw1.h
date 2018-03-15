#ifndef HW_H
#define HW_H

#include "const.h"

#endif

extern long double space;
extern char *buffer;
extern int buffer_number;
extern int write_flag;
extern int read_flag;

extern long double space1;
extern char *decode_char;

int print_buffer();

void clean_buffer();

void write_buffer(char temp);

void print_buffer_decode();
