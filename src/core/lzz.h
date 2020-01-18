//
// Created by zbh on 2019/12/26.
//

#ifndef LZZ3_LZZ_H
#define LZZ3_LZZ_H

extern int ERROR_CODE;

extern int LAB_SIZE;
extern int WINDOW_SIZE;
extern int DICT_SIZE;

unsigned char *compress(unsigned char *plain_text, unsigned long text_len, unsigned long *res_len, int level);

void set_window(int window_size, int lab_size);

void free_hashtable();

unsigned int write_dis_bits(unsigned int dis, unsigned long *bits, unsigned int *bit_pos);

#endif //LZZ3_LZZ_H
