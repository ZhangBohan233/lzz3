//
// Created by zbh on 2020/1/15.
//

#include <stdio.h>
#include <stdlib.h>
#include "huffman_d.h"
#include "huffman_c.h"

/*
 * All of these functions returns the read length
 */
unsigned long recover_length_small(const unsigned char *text) {
    for (unsigned int i = 0; i < 8; ++i) {
        unsigned char c = text[i];
        unsigned int len1 = c >> 4u;
        unsigned int len2 = c & 0x0fu;
        CODE_LENGTH_SMALL_D[i << 1u] = len1;
        CODE_LENGTH_SMALL_D[(i << 1u) + 1] = len2;
    }

    return 8;
}

unsigned long recover_length_big(const unsigned char *text) {
    unsigned int res_i = 0;
    unsigned int i = 0;
    unsigned long bits = 0;
    unsigned int bit_pos = 0;
    int count = 0;
    unsigned int repeat = 0;
    while (res_i < 273) {
//        printf("%d ", res_i);
        if (bit_pos < 5) {
            bits <<= 8u;
            bits |= text[i++];
            bit_pos += 8;
        }
        unsigned int latest = (bits >> (bit_pos - 5)) & 0x1fu;
//        printf("%d ", latest);
        bit_pos -= 5;
        if (latest == 30) {
            count = 1;
        } else if (latest == 31) {
            count = 2;
        } else {
            if (count == 0) {
                CODE_LENGTH_BIG_D[res_i++] = latest;
            } else if (count == 1) {
                repeat += latest;
//                printf("rep %d ", repeat);
                for (int j = 0; j < repeat; ++j) {
                    CODE_LENGTH_BIG_D[res_i++] = 0;
                }
                count = 0;
                repeat = 0;
            } else {
                repeat = latest << 5u;
                count = 1;
            }
        }
    }
//    print_array(CODE_LENGTH_BIG_D, 273);
    return i;
}

void recover_canonical_code_small() {
    generate_canonical_code(MAP_SMALL, CODE_LENGTH_SMALL_D, 16);
//    print_array(CODE_LENGTH_SMALL_D, 16);
//    print_canonical_code(MAP_SMALL, CODE_LENGTH_SMALL_D, 16);
    for (unsigned int i = 0; i < 16; ++i) {
        unsigned int code = MAP_SMALL[i];
        if (CODE_LENGTH_SMALL_D[i] > 0)
            INVERSE_MAP_SMALL[code] = i + 1;  // make 0 the empty checker
    }
//    printf("gg\n");
}

void recover_canonical_code_big() {
    generate_canonical_code(MAP_BIG, CODE_LENGTH_BIG_D, 273);

//    printf("ff\n");

//    for (unsigned int i = 0; i < 273; ++i) {
//        unsigned int len = CODE_LENGTH_BIG_D[i];
//        if (len > 16) {
//            printf("fuck %d\n", len);
//            exit(3);
//        }
//        unsigned int code = MAP_BIG[i];
//        if (len > 0)
//            INVERSE_MAP_BIG[code] = i + 1;  // make 0 the empty checker
//    }

    for (unsigned int i = 0; i < 273; ++i) {
        unsigned int len = CODE_LENGTH_BIG_D[i];
        if (len > 0) {
            unsigned int code = MAP_BIG[i];
            if (len < 8) {
                unsigned int sup_len = 8 - len;
                unsigned int sup_pow = 1u << sup_len;
                unsigned int res = code << sup_len;
                for (unsigned int j = 0; j < sup_pow; ++j) {
                    MAP_BIG_SHORT[res + j] = i + 1;  // 0 reserved for not found
                }
            } else if (len == 8) {
                MAP_BIG_SHORT[code] = i + 1;  // 0 reserved for not found
            } else if (len < 16) {
                unsigned int sup_len = 16 - len;
                unsigned int sup_pow = 1u << sup_len;
                unsigned int res = code << sup_len;
                for (unsigned int j = 0; j < sup_pow; ++j) {
                    MAP_BIG_LONG[res + j] = i;
                }
            } else if (len == 16) {
                MAP_BIG_LONG[code] = i;
            } else {
                printf("Code length exceed 16\n");
                exit(3);
            }
        }
    }
}
