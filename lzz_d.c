//
// Created by zbh on 2019/12/26.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lzz_d.h"
#include "lib.h"
#include "huffman_d.h"
#include "huffman_c.h"

#define read_bits(least_bit_pos, bits, bit_pos, index) while (bit_pos < least_bit_pos) {\
    bit_pos += 8;  \
    bits <<= 8u;  \
    bits |= cmp_text[index++];  \
}

#define get_and_er(bit_len) {   \
    and_er = 1u;                \
    for (unsigned int j = 1; j < bit_len; ++j) { \
        and_er <<= 1u;          \
        and_er |= 1u;           \
    }                           \
}

const int MIN_LEN_D = 3;
const int MIN_DIS_D = 1;

//unsigned char *CMP_TEXT;



unsigned char *uncompress(unsigned char *cmp_text, unsigned long *result_len_ptr) {
//    CMP_TEXT = cmp_text;

    unsigned long text_index = 12;
    unsigned long orig_len = bytes_to_int_32(cmp_text);
    unsigned long dis_head_index = bytes_to_int_32(cmp_text + 4);
    unsigned long body_index = bytes_to_int_32(cmp_text + 8);
    *result_len_ptr = orig_len;
    text_index += recover_length_small(cmp_text + text_index);
    text_index += recover_length_big(cmp_text + text_index);

//    printf("%ld\n", text_index);

    recover_canonical_code_small();
    recover_canonical_code_big();

    unsigned char *ump_text = malloc(orig_len);

    unsigned long out_index = 0;

    unsigned int len_head;
    unsigned int dis_head;

    unsigned int len;
    unsigned int dis;

    unsigned long len_bits = 0;
    unsigned int len_pos = 0;
    unsigned long dis_bits = 0;
    unsigned int dis_pos = 0;
    unsigned long body_bits = 0;
    unsigned int body_pos = 0;

//    unsigned int len_code;
//    unsigned int lhl;
    unsigned int and_er = 0;

    unsigned int base;
    unsigned int bit_len;

    unsigned int dhl;
    unsigned int dis_code;
    unsigned int code_len;

    while (out_index < orig_len) {
        /// another
//        read_bits(16, len_bits, len_pos, text_index)
//        lhl = 1;
//        and_er = 1u;
//        while (1) {
//            len_code = (len_bits >> (len_pos - lhl)) & and_er;
//            len_head = INVERSE_MAP_BIG[len_code];
//            if (len_head > 0 && CODE_LENGTH_BIG_D[len_head - 1] == lhl) {
//                len_head -= 1;
//                break;
//            }
//            lhl++;
//            and_er <<= 1u;
//            and_er |= 1u;
//        }
//        len_pos -= lhl;

        /// end another

        read_bits(8, len_bits, len_pos, text_index)
        unsigned int index = (len_bits >> (len_pos - 8)) & 0xffu;
        len_pos -= 8;
        len_head = MAP_BIG_SHORT[index];

        if (len_head == 0) {
            read_bits(8, len_bits, len_pos, text_index)
            index <<= 8u;
            index |= ((len_bits >> (len_pos - 8)) & 0xffu);
            len_pos -= 8;
            len_head = MAP_BIG_LONG[index];
            code_len = CODE_LENGTH_BIG_D[len_head];
            len_pos += (16 - code_len);
        } else {
            len_head -= 1;
            code_len = CODE_LENGTH_BIG_D[len_head];
            len_pos += (8 - code_len);
        }
//
//        printf("lh: %u ", len_head);

        if (len_head < 256) {  // literal
            ump_text[out_index++] = len_head;
//            printf("lit: %d, ", len_head);
        } else if (len_head == 256) {   // reach the end sig
            break;
        } else {  // length head
//            printf("%d ", len_head);
            bit_len = 0;
            switch (len_head) {
                case 257:
                case 258:
                case 259:
                case 260:
                    base = len_head - 257;
                    and_er = 0u;
                    break;
                case 261:
                    base = 4;
                    bit_len = 1;
                    and_er = 0b1u;
                    break;
                case 262:
                    base = 6;
                    bit_len = 1;
                    and_er = 0b1u;
                    break;
                case 263:
                    base = 8;
                    bit_len = 2;
                    and_er = 0b11u;
                    break;
                case 264:
                    base = 12;
                    bit_len = 2;
                    and_er = 0b11u;
                    break;
                case 265:
                    base = 16;
                    bit_len = 3;
                    and_er = 0b111u;
                    break;
                case 266:
                    base = 24;
                    bit_len = 3;
                    and_er = 0b111u;
                    break;
                case 267:
                    base = 32;
                    bit_len = 4;
                    and_er = 0x0fu;
                    break;
                case 268:
                    base = 48;
                    bit_len = 4;
                    and_er = 0x0fu;
                    break;
                case 269:
                    base = 64;
                    bit_len = 5;
                    and_er = 0x1fu;
                    break;
                case 270:
                    base = 96;
                    bit_len = 5;
                    and_er = 0x1fu;
                    break;
                case 271:
                    base = 128;
                    bit_len = 6;
                    and_er = 0x3fu;
                    break;
                case 272:
                    base = 192;
                    bit_len = 6;
                    and_er = 0x3fu;
                    break;
                default:
                    printf("Unknown length head");
                    exit(3);
            }
            read_bits(bit_len, body_bits, body_pos, body_index)
            len = (body_bits >> (body_pos - bit_len)) & and_er;
            body_pos -= bit_len;
            len += MIN_LEN_D + base;
//            printf("%d ", len);

            /// now read dis
            read_bits(16, dis_bits, dis_pos, dis_head_index)
            dhl = 1;
            and_er = 1u;

            while (1) {
                dis_code = (dis_bits >> (dis_pos - dhl)) & and_er;
                dis_head = INVERSE_MAP_SMALL[dis_code];
                if (dis_head > 0 && CODE_LENGTH_SMALL_D[dis_head - 1] == dhl) {
                    dis_head -= 1;
                    break;
                }
                dhl++;
                and_er <<= 1u;
                and_er |= 1u;
                if (dhl > 16) exit(9);
            }
            dis_pos -= dhl;

            if (dis_head == 0) {
                dis = MIN_DIS_D;
            } else {
                bit_len = dis_head - 1;
                base = 1u << bit_len;
                read_bits(bit_len, body_bits, body_pos, body_index)
                get_and_er(bit_len)
                dis = (body_bits >> (body_pos - bit_len)) & and_er;
                dis += MIN_DIS_D + base;
                body_pos -= bit_len;
            }
//            printf(" %u %u %u, ", dis_head, bit_len, dis);

//            printf("len %d dis %d lh %d, ", len, dis, len_head);
            unsigned long begin_index = out_index - dis;
            unsigned long end_index = begin_index + len;
            if (end_index <= out_index) {
                memcpy(ump_text + out_index, ump_text + begin_index, len);
            } else {  // overlap
                unsigned int p = 0;
                unsigned int overlap = out_index - begin_index;

                while (p < len) {
                    memcpy(ump_text + out_index + p, ump_text + begin_index, min(overlap, len - p));
                    p += overlap;
                }
            }
            out_index += len;
        }
    }

    return ump_text;
}
