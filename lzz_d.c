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

    while (1) {
        read_bits(8, len_bits, len_pos, text_index)
        unsigned int index = (len_bits >> (len_pos - 8)) & 0xffu;
        len_pos -= 8;
        len_head = MAP_BIG_SHORT[index];
        unsigned int code_len = CODE_LENGTH_BIG_D[len_head];
//        printf("%d %d %d ", index, lit, code_len);
        if (code_len == 0) {
            read_bits(8, len_bits, len_pos, text_index)
            index <<= 8u;
            index |= ((len_bits >> (len_pos - 8)) & 0xffu);
            len_pos -= 8;
            len_head = MAP_BIG_LONG[index];
            code_len = CODE_LENGTH_BIG_D[len_head];
            len_pos += (16 - code_len);
        } else {
            len_pos += (8 - code_len);
        }

        if (len_head < 256) {  // literal
            ump_text[out_index++] = len_head;
        } else if (len_head == 256) {   // reach the end sig
            break;
        } else {  // length head
//            printf("%d ", len_head);
            unsigned int base;
            unsigned int and_er = 0;
            unsigned int bit_len = 0;
            switch (len_head) {
                case 257:
                case 258:
                case 259:
                case 260:
                    base = len_head - 257;
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
            read_bits(bit_len, body_bits, body_pos, body_index);
            len = (body_bits >> (body_pos - bit_len)) & and_er;
            body_pos -= bit_len;
            len += MIN_LEN_D + base;
//            printf("%d ", len);

            /// now read dis
            read_bits(16, dis_bits, dis_pos, dis_head_index)
            unsigned int dhl = 1;
            and_er = 1u;

            unsigned int dis_code;
//            printf("%lu, ", dis_bits >> 8u);
//            print_binary(dis_bits, 8);

            while (1) {
                dis_code = (dis_bits >> (dis_pos - dhl)) & and_er;
                for (unsigned int j = 0; j < 16; ++j) {
                    if (dis_code == MAP_SMALL[j] && dhl == CODE_LENGTH_SMALL_D[j]) {
//                        printf("%u %u\n", dis_code, dhl);
                        dis_head = j;
                        goto end;
                    }
                }
                dhl++;
                and_er <<= 1u;
                and_er |= 1u;
            }
            end:
            dis_pos -= dhl;

            if (dis_head == 0) {
//                base = 0;
//                bit_len = 0;
//                and_er = 0;
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

//            printf("dis: %d, len: %d; ", dis, len);
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

//    unsigned long orig_file_len = bytes_to_int_32(cmp_text);
//    *result_len_ptr = orig_file_len;
//
//    unsigned long index = 4;
//    unsigned long result_index = 0;
//
//    unsigned char *result = malloc(orig_file_len);
//
//    unsigned long bits = 0;
//    unsigned int bit_pos = 0;
//    unsigned int flag;
//
//    unsigned char literal;
//    unsigned int len_head;
//    unsigned int len;
//    unsigned int dis_head;
//    unsigned int dis;
//
//    unsigned int bit_len;
//    unsigned int adder;
//    unsigned int and_er;
//
//    unsigned long begin_index, end_index;
//
//    while (result_index < orig_file_len) {
//        read_bits(1)
//        flag = (bits >> (bit_pos - 1)) & 0b1u;
//        bit_pos--;
//        if (flag == 0) {  // is literal
//            read_bits(8)
//            literal = bits >> (bit_pos - 8);
//            result[result_index++] = literal;
//            bit_pos -= 8;
//        } else {
//            read_bits(2)
//            len_head = (bits >> (bit_pos - 2)) & 0b11u;
//            bit_pos -= 2;
//            calculate_len(len_head)
//
//            read_bits(bit_len)
//            len = (bits >> (bit_pos - bit_len)) & and_er;
//            bit_pos -= bit_len;
//            len += adder + MIN_LEN_D;
//
//            read_bits(3)
//            dis_head = (bits >> (bit_pos - 3)) & 0b111u;
//            bit_pos -= 3;
//            calculate_dis(dis_head)
//
//            read_bits(bit_len)
//            dis = (bits >> (bit_pos - bit_len)) & and_er;
//            bit_pos -= bit_len;
//            dis += adder + MIN_DIS_D;
//
////            printf("len: %u, dis: %u | ", len, dis);
//
//            begin_index = result_index - dis;
//            end_index = begin_index + len;
//            if (end_index <= result_index) {
//                memcpy(result + result_index, result + begin_index, len);
//            } else {  // overlap
//                unsigned int p = 0;
//                unsigned int overlap = result_index - begin_index;
//
//                while (p < len) {
//                    memcpy(result + result_index + p, result + begin_index, min(overlap, len - p));
//                    p += overlap;
//                }
//            }
//            result_index += len;
//        }
//    }
//
//    return result;
}
