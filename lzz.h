//
// Created by zbh on 2019/12/26.
//

#ifndef LZZ3_LZZ_H
#define LZZ3_LZZ_H

extern int ERROR_CODE;

unsigned char *compress(unsigned char *plain_text, unsigned long text_len, unsigned long *res_len);

void free_hashtable();

#endif //LZZ3_LZZ_H
