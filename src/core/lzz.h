//
// Created by zbh on 2019/12/26.
//

#ifndef LZZ3_LZZ_H
#define LZZ3_LZZ_H

extern int ERROR_CODE;

unsigned char *compress(unsigned char *plain_text, unsigned long text_len, unsigned long *res_len, int level);

void free_hashtable();

unsigned int write_dis_bits(unsigned int dis, unsigned long *bits, unsigned int *bit_pos);

#endif //LZZ3_LZZ_H
