//
// Created by zbh on 2019/12/3.
//

#ifndef TPL3_LIB_H
#define TPL3_LIB_H

#include <stdint.h>

unsigned char *read_file(char *file_name, unsigned long *length_ptr);

int write_file(char *file_name, unsigned char *text, unsigned long length);

void int_to_bytes_16(unsigned char *b, unsigned long i);

void int_to_bytes_32(unsigned char *b, unsigned long i);

unsigned long bytes_to_int_32(const unsigned char *b);

/*
 * Returns 0 if <s> does not ends with <pattern>, 1 otherwise
 */
int end_with(const char *s, const char *pattern);

#endif //TPL3_LIB_H
