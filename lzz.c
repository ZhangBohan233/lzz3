//
// Created by zbh on 2019/12/26.
//

//#include <stdio.h>
#include <stdlib.h>
//#include "lzz.h"
#include "lib.h"
#include "array_deque.h"

#define hash(b0, b1) (((unsigned int) b0 << 8u) | b1)
//#define hash(b0, b1, b2) (((unsigned int) b0 & 0x1fu) << 10u | ((unsigned int) b1 & 0x1fu) << 5u | b2 & 0x1fu)

#define flush_bits while (bit_pos >= 8) {   \
    bit_pos -= 8;                           \
    temp = (bits >> bit_pos) & 0xffu;       \
    output[result_index++] = temp;          \
}

const int MIN_LEN = 3;
const int MIN_DIS = 1;

int ERROR_CODE = 0;

int LAB_SIZE = 255;
int WINDOW_SIZE = 32767;
int DICT_SIZE = 32767 - 256;

ArrayDeque *HASH_TABLE[65536];

unsigned char *TEXT;
unsigned long TEXT_LENGTH;

void add_slider(unsigned int hash_code, unsigned long pos) {
    ArrayDeque *positions = HASH_TABLE[hash_code];
    if (positions == NULL) {
        positions = create_adq();
        HASH_TABLE[hash_code] = positions;
    }
    adq_add_last(positions, pos);
}

void fill_slider(unsigned long prev_i, unsigned long i) {
    unsigned int last_hash = 65536;
    unsigned int repeat_count = 0;
    for (unsigned long j = prev_i; j < i; ++j) {
        unsigned int hash_code = hash(TEXT[j], TEXT[j + 1]);
        if (hash_code == last_hash) repeat_count++;
        else {
            if (repeat_count > 0) {
                repeat_count = 0;
                add_slider(last_hash, j - 1);
            }
            last_hash = hash_code;
            add_slider(hash_code, j);
        }
    }
}

void longest_match(unsigned long index, unsigned int *dis_ptr, unsigned int *len_ptr) {
    unsigned int hash_code = hash(TEXT[index], TEXT[index + 1]);
    ArrayDeque *positions = HASH_TABLE[hash_code];
    if (positions == NULL) {
        *len_ptr = 0;
        *dis_ptr = 0;
        return;
    }

    unsigned int len;
    unsigned long pos;
    unsigned int longest = 2;
    unsigned long pos_of_longest = positions->array[(positions->tail - 1) & 15u];
    unsigned int abp = adq_begin_pos(positions);
    unsigned long window_begin = index > DICT_SIZE ? index - DICT_SIZE : 0;

    for (unsigned int i = positions->tail; i > abp; --i) {
        len = 2;
        pos = positions->array[(i - 1u) & 15u];
        if (pos < window_begin) break;  // the adq is sorted from small to big. Since the loop is reversed, it loops
        // from big to small. If current is smaller than window, then the rest must also out of window
        while (len < LAB_SIZE &&
               index + len < TEXT_LENGTH &&
               TEXT[pos + len] == TEXT[index + len]) {
            len++;
        }
        if (len > longest) {
            longest = len;
            pos_of_longest = pos;
        }
    }

    *dis_ptr = index - pos_of_longest;
    *len_ptr = longest;
}

void validate_window() {
    if (TEXT_LENGTH < WINDOW_SIZE) {
        WINDOW_SIZE = (int) TEXT_LENGTH;
        DICT_SIZE = WINDOW_SIZE;
    }
    if (WINDOW_SIZE < LAB_SIZE) {
        LAB_SIZE = WINDOW_SIZE;
    }
}

void write_length_bit(unsigned int len, unsigned long *bits, unsigned int *bit_pos) {
    unsigned int out_standing_len = len - MIN_LEN;
    unsigned int head;
    unsigned int content = 0;
    unsigned int bit_len = 0;

    if (out_standing_len == 0) {
        head = 0;
    } else if (out_standing_len < 5) {
        head = 1;
        content = out_standing_len - 1;
        bit_len = 2;
    } else if (out_standing_len < 21) {
        head = 2;
        content = out_standing_len - 5;
        bit_len = 4;
    } else {
        head = 3;
        content = out_standing_len - 21;
        bit_len = 8;
    }
    unsigned long bits_real = *bits;
    bits_real <<= 2u;
    bits_real |= head;
    bits_real <<= bit_len;
    bits_real |= content;
    *bits = bits_real;
    (*bit_pos) += (bit_len + 2);
}

void write_dis_bits(unsigned int dis, unsigned long *bits, unsigned int *bit_pos) {
    unsigned int out_standing_dis = dis - MIN_DIS;
    unsigned int head;
    unsigned int content;
    unsigned int bit_len;
    if (out_standing_dis < 32) {
        head = 0;
        content = out_standing_dis;
        bit_len = 5;
    } else if (out_standing_dis < 64) {
        head = 1;
        content = out_standing_dis - 32;
        bit_len = 5;
    } else if (out_standing_dis < 128) {
        head = 2;
        content = out_standing_dis - 64;
        bit_len = 6;
    } else if (out_standing_dis < 256) {
        head = 3;
        content = out_standing_dis - 128;
        bit_len = 7;
    } else if (out_standing_dis < 512) {
        head = 4;
        content = out_standing_dis - 256;
        bit_len = 8;
    } else if (out_standing_dis < 1536) {
        head = 5;
        content = out_standing_dis - 512;
        bit_len = 10;
    } else if (out_standing_dis < 5632) {
        head = 6;
        content = out_standing_dis - 1536;
        bit_len = 12;
    } else {
        head = 7;
        content = out_standing_dis - 5632;
        bit_len = 16;
    }
    unsigned long bits_real = *bits;
    bits_real <<= 3u;
    bits_real |= head;
    bits_real <<= bit_len;
    bits_real |= content;
    *bits = bits_real;
    (*bit_pos) += (bit_len + 3);
}

unsigned char *compress(unsigned char *plain_text, unsigned long text_len, unsigned long *res_len) {
    TEXT = plain_text;
    TEXT_LENGTH = text_len;

    validate_window();

    unsigned char *output = malloc(text_len * 2 + 4);
    int_to_bytes_32(output, text_len);  // record the orig length

    unsigned long result_index = 4;

    unsigned int bit_pos = 0;
    unsigned long bits = 0;
    unsigned int temp = 0;

    unsigned long i = 0;
    unsigned long prev_i;
    unsigned int dis;
    unsigned int len;
    while (i < text_len - 3) {
        longest_match(i, &dis, &len);
        prev_i = i;

        if (len < MIN_LEN) {

            bit_pos += 9;
            bits <<= 9u;  // write flag: 0
            bits |= plain_text[i];

            flush_bits

            ++i;
        } else {
            bits <<= 1u;
            bits |= 1u;  // write flag: 1
            bit_pos += 1;

            write_length_bit(len, &bits, &bit_pos);
            write_dis_bits(dis, &bits, &bit_pos);

            flush_bits

            i += len;
        }

//        if (i > text_len) break;
        fill_slider(prev_i, i);
    }
    for (; i < text_len; ++i) {

        bit_pos += 9;
        bits <<= 9u;
        bits |= plain_text[i];
        flush_bits
    }

    if (bit_pos > 0) {  // write the last bits
        bits <<= (8 - bit_pos);
        output[result_index++] = bits;
    }

    *res_len = result_index;

    return output;
}

void free_hashtable() {
    for (int i = 0; i < 65536; ++i) {
        free(HASH_TABLE[i]);
    }
}
