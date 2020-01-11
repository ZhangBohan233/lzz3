//
// Created by zbh on 2019/12/26.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lzz_d.h"
#include "lib.h"

#define read_bits(least_bit_pos) while (bit_pos < least_bit_pos) {\
    bit_pos += 8;  \
    bits <<= 8u;  \
    bits |= CMP_TEXT[index++];  \
}

#define calculate_len(len_head) {        \
    switch (len_head) {                  \
        case 0:                      \
            bit_len = 0;             \
            adder = 0;               \
            and_er = 0;              \
            break;                   \
        case 1:                      \
            bit_len = 2;             \
            adder = 1;               \
            and_er = 0b11u;          \
            break;                   \
        case 2:                      \
            bit_len = 4;             \
            adder = 5;               \
            and_er = 0x0fu;          \
            break;                   \
        default:                     \
            bit_len = 8;             \
            adder = 21;              \
            and_er = 0xffu;          \
            break;                   \
    }                                \
}

#define calculate_dis(dis_head) {   \
    switch (dis_head) {             \
        case 0:                     \
            bit_len = 5;            \
            adder = 0;              \
            and_er = 0x1fu;         \
            break;                  \
        case 1:                     \
            bit_len = 5;            \
            adder = 32;             \
            and_er = 0x1fu;         \
            break;                  \
        case 2:                     \
            bit_len = 6;            \
            adder = 64;             \
            and_er = 0x3fu;         \
            break;                  \
        case 3:                     \
            bit_len = 7;            \
            adder = 128;            \
            and_er = 0x7fu;         \
            break;                  \
        case 4:                     \
            bit_len = 8;            \
            adder = 256;            \
            and_er = 0xffu;         \
            break;                  \
        case 5:                     \
            bit_len = 10;           \
            adder = 512;            \
            and_er = 0x3ffu;        \
            break;                  \
        case 6:                     \
            bit_len = 12;           \
            adder = 1536;           \
            and_er = 0xfffu;        \
            break;                  \
        default:                    \
            bit_len = 16;           \
            adder = 5632;           \
            and_er = 0xffffu;       \
            break;                  \
    }                               \
}

const int MIN_LEN_D = 3;
const int MIN_DIS_D = 1;

unsigned char *CMP_TEXT;

unsigned char *uncompress(unsigned char *cmp_text, unsigned long *result_len_ptr) {
    CMP_TEXT = cmp_text;

    unsigned long orig_file_len = bytes_to_int_32(cmp_text);
    *result_len_ptr = orig_file_len;

    unsigned long index = 4;
    unsigned long result_index = 0;

    unsigned char *result = malloc(orig_file_len);

    unsigned long bits = 0;
    unsigned int bit_pos = 0;
    unsigned int flag;

    unsigned char literal;
    unsigned int len_head;
    unsigned int len;
    unsigned int dis_head;
    unsigned int dis;

    unsigned int bit_len;
    unsigned int adder;
    unsigned int and_er;

    unsigned long begin_index, end_index;

    while (result_index < orig_file_len) {
        read_bits(1)
        flag = (bits >> (bit_pos - 1)) & 0b1u;
        bit_pos--;
        if (flag == 0) {  // is literal
            read_bits(8)
            literal = bits >> (bit_pos - 8);
            result[result_index++] = literal;
            bit_pos -= 8;
        } else {
            read_bits(2)
            len_head = (bits >> (bit_pos - 2)) & 0b11u;
            bit_pos -= 2;
            calculate_len(len_head)

            read_bits(bit_len)
            len = (bits >> (bit_pos - bit_len)) & and_er;
            bit_pos -= bit_len;
            len += adder + MIN_LEN_D;

            read_bits(3)
            dis_head = (bits >> (bit_pos - 3)) & 0b111u;
            bit_pos -= 3;
            calculate_dis(dis_head)

            read_bits(bit_len)
            dis = (bits >> (bit_pos - bit_len)) & and_er;
            bit_pos -= bit_len;
            dis += adder + MIN_DIS_D;

//            printf("len: %u, dis: %u | ", len, dis);

            begin_index = result_index - dis;
            end_index = begin_index + len;
            if (end_index <= result_index) {
                memcpy(result + result_index, result + begin_index, len);
            } else {  // overlap
                unsigned int p = 0;
                unsigned int overlap = result_index - begin_index;

                while (p < len) {
                    memcpy(result + result_index + p, result + begin_index, min(overlap, len - p));
                    p += overlap;
                }
            }
            result_index += len;
        }
    }

    return result;
}
