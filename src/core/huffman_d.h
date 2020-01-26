//
// Created by zbh on 2020/1/15.
//

#include "huffman_c.h"

#ifndef LZZ3_HUFFMAN_D_H
#define LZZ3_HUFFMAN_D_H

unsigned int CODE_LENGTH_SMALL_D[SMALL];
unsigned int CODE_LENGTH_BIG_D[BIG];

unsigned int MAP_SMALL[SMALL];  // Perform naive algorithm since this map is small
unsigned int MAP_BIG[BIG];

//unsigned int INVERSE_MAP_BIG[65536];
unsigned char *INVERSE_MAP_SMALL;

unsigned int MAX_CODE_LEN;

unsigned short MAP_BIG_SHORT[256];
unsigned short *MAP_BIG_LONG;

/*
 * All of the following 2 functions returns the read length.
 */
unsigned long recover_length_small(const unsigned char *text);

unsigned long recover_length_big(const unsigned char *text);

void recover_canonical_code_small();

void recover_canonical_code_big();

void free_tables();

#endif //LZZ3_HUFFMAN_D_H
