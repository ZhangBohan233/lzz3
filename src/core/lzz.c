//
// Created by zbh on 2019/12/26.
//
// Structure: len/literal head, dis head, len/dis bits
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lzz.h"
#include "lib.h"
#include "array_deque.h"
#include "huffman_c.h"

#define HASH_SIZE 65536

#define hash(b0, b1) (((unsigned int) b0 << 8u) | b1)
//#define hash(b0, b1, b2) (((unsigned int) b0 & 0x1fu) << 10u | ((unsigned int) b1 & 0x1fu) << 5u | b2 & 0x1fu)

#define flush_bits while (bit_pos >= 8) {   \
    bit_pos -= 8;                           \
    temp = (bits >> bit_pos) & 0xffu;       \
    body_output[body_i++] = temp;          \
}

const int MIN_LEN = 3;
const int MIN_DIS = 1;

int ERROR_CODE = 0;

int LAB_SIZE = 255;
int WINDOW_SIZE = 32767;
int DICT_SIZE = 32767 - 256;

ArrayDeque *HASH_TABLE;
unsigned long *ADQ_ARRAY_POOL;

unsigned long *SIMPLE_HASH_TABLE;

unsigned char *TEXT;
unsigned long TEXT_LENGTH;


unsigned int simple_search(unsigned long index, unsigned int *dis_ptr, unsigned int *len_ptr) {
    unsigned int hash_code = hash(TEXT[index], TEXT[index + 1]);
    unsigned long pos = SIMPLE_HASH_TABLE[hash_code];
    if (pos == 0) {  // not a match
        *len_ptr = 0;
        *dis_ptr = 0;
        return 0;
    }
    pos -= 1;  // indices were added by 1 when filling in hash table

    unsigned int len = 2;
    unsigned long window_begin = index > DICT_SIZE ? index - DICT_SIZE : 0;

    if (pos < window_begin) {  // match was too far away
        *len_ptr = 0;
        *dis_ptr = 0;
        return 0;
    }

    while (len < LAB_SIZE &&
           index + len < TEXT_LENGTH &&
           TEXT[pos + len] == TEXT[index + len]) {
        len++;
    }

    *dis_ptr = index - pos;
    *len_ptr = len;
    return 0;
}

void simple_fill_slider(unsigned long prev_i, unsigned long i) {
    unsigned int last_hash = 65536;
    unsigned int repeat_count = 0;
    for (unsigned long j = prev_i; j < i; ++j) {
        unsigned int hash_code = hash(TEXT[j], TEXT[j + 1]);
        if (hash_code == last_hash) repeat_count++;
        else {
            if (repeat_count > 0) {
                repeat_count = 0;
                SIMPLE_HASH_TABLE[last_hash] = j;  // j == 0 is impossible
            }
            last_hash = hash_code;
            SIMPLE_HASH_TABLE[hash_code] = j + 1;  // 0 indicates does not contain so index begins with 1
        }
    }
}

void add_slider(unsigned int hash_code, unsigned long pos) {
    ArrayDeque *positions = &HASH_TABLE[hash_code];
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

unsigned int search_one_step(unsigned long index, unsigned int *dis_ptr, unsigned int *len_ptr) {
    unsigned int hash_code = hash(TEXT[index], TEXT[index + 1]);
    ArrayDeque *positions = &HASH_TABLE[hash_code];
    if (positions == NULL) {
        *len_ptr = 0;
        *dis_ptr = 0;
        return 0;
    }

    unsigned int len;
    unsigned long pos;
    unsigned int longest = 2;
    unsigned long pos_of_longest = positions->array[(positions->tail - 1) & ADQ_AND];
    unsigned int abp = adq_begin_pos(positions);
    unsigned long window_begin = index > DICT_SIZE ? index - DICT_SIZE : 0;

    for (unsigned int i = positions->tail; i > abp; --i) {
        len = 2;
        pos = positions->array[(i - 1u) & ADQ_AND];
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

    return 0;
}

unsigned int search_two_steps(unsigned long index, unsigned int *dis_ptr, unsigned int *len_ptr) {
    unsigned int dis1, len1, dis2, len2;
    search_one_step(index, &dis1, &len1);
    search_one_step(index + 1, &dis2, &len2);
    if (len2 > len1 + 1) {
        *dis_ptr = dis2;
        *len_ptr = len2;
        return 1;
    } else {
        *dis_ptr = dis1;
        *len_ptr = len1;
        return 0;
    }
}

unsigned int search_mul_steps(unsigned long index, unsigned int *dis_ptr, unsigned int *len_ptr) {
    unsigned int skip = 1;
    unsigned int dis1, len1;
    search_one_step(index, &dis1, &len1);
    unsigned int dis2, len2;
    while (1) {
        search_one_step(index + skip, &dis2, &len2);
        if (len2 >= len1 + skip) {
            dis1 = dis2;
            len1 = len2;
            skip++;
        } else {
            *dis_ptr = dis1;
            *len_ptr = len1;
            return skip - 1;
        }
    }
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

void set_window(int window_size, int lab_size) {
    WINDOW_SIZE = window_size;
    LAB_SIZE = lab_size;
    DICT_SIZE = WINDOW_SIZE - LAB_SIZE - 1;
}

unsigned short write_length_bit(unsigned int len, unsigned long *bits, unsigned int *bit_pos) {
    unsigned int out_standing_len = len - MIN_LEN;
    unsigned short head;
    unsigned int content = 0;
    unsigned int bit_len = 0;

//    printf("%d ", len);

    if (out_standing_len == 0) {
        head = 257;
    } else if (out_standing_len == 1) {
        head = 258;
    } else if (out_standing_len == 2) {
        head = 259;
    } else if (out_standing_len == 3) {
        head = 260;
    } else if (out_standing_len < 6) {
        head = 261;
        bit_len = 1;
        content = out_standing_len - 4;
    } else if (out_standing_len < 8) {
        head = 262;
        bit_len = 1;
        content = out_standing_len - 6;
    } else if (out_standing_len < 12) {
        head = 263;
        bit_len = 2;
        content = out_standing_len - 8;
    } else if (out_standing_len < 16) {
        head = 264;
        bit_len = 2;
        content = out_standing_len - 12;
    } else if (out_standing_len < 24) {
        head = 265;
        bit_len = 3;
        content = out_standing_len - 16;
    } else if (out_standing_len < 32) {
        head = 266;
        bit_len = 3;
        content = out_standing_len - 24;
    } else if (out_standing_len < 48) {
        head = 267;
        bit_len = 4;
        content = out_standing_len - 32;
    } else if (out_standing_len < 64) {
        head = 268;
        bit_len = 4;
        content = out_standing_len - 48;
    } else if (out_standing_len < 96) {
        head = 269;
        bit_len = 5;
        content = out_standing_len - 64;
    } else if (out_standing_len < 128) {
        head = 270;
        bit_len = 5;
        content = out_standing_len - 96;
    } else if (out_standing_len < 192) {
        head = 271;
        bit_len = 6;
        content = out_standing_len - 128;
    } else if (out_standing_len < 256) {
        head = 272;
        bit_len = 6;
        content = out_standing_len - 192;
    } else {
        printf("Error while writing length\n");
        exit(1);
    }
//    printf("%d ", head);
    unsigned long bits_real = *bits;
    bits_real <<= bit_len;
    bits_real |= content;
    *bits = bits_real;
    (*bit_pos) += bit_len;

    return head;
}

unsigned char write_dis_bits(unsigned int dis, unsigned long *bits, unsigned int *bit_pos) {
    unsigned int out_standing_dis = dis - MIN_DIS;
    unsigned char head;
    unsigned int content = 0;
    unsigned int bit_len = 0;
    if (out_standing_dis == 0) {
        head = 0;
    } else {
        head = 0;
        unsigned int osd = out_standing_dis;
        while (osd != 0) {
            osd >>= 1u;
            head++;
        }
        bit_len = head - 1;
        content = out_standing_dis - (1u << bit_len);
//        printf("%d %d %d, ", head, bit_len, dis);
        // head = log2(osd) + 1
        // bit_len = log2(osd)
        // content = remaining
        // maximum dis: 8388607
        // maximum head: 23
    }
    unsigned long bits_real = *bits;
    bits_real <<= bit_len;
    bits_real |= content;
    *bits = bits_real;
    (*bit_pos) += bit_len;

    return head;
}

unsigned char *compress_content(unsigned char *plain_text, unsigned long text_len, unsigned long *res_len,
                                unsigned int (*search_fn)(unsigned long, unsigned int *, unsigned int *),
                                void (*slider_fn)(unsigned long, unsigned long)) {
    TEXT = plain_text;
    TEXT_LENGTH = text_len;

    validate_window();

    unsigned short *len_lit_heads = malloc((text_len + 1) * sizeof(unsigned short));
    unsigned char *dis_heads = malloc(text_len / 3 + 1);  // maximum match will be less than / 3
    unsigned char *body_output = malloc(text_len);

    unsigned long len_lit_i = 0;
    unsigned long dis_head_i = 0;
    unsigned long body_i = 0;

    unsigned int bit_pos = 0;
    unsigned long bits = 0;
    unsigned int temp = 0;

    unsigned short len_head;
    unsigned char dis_head;

    unsigned long i = 0;
    unsigned long prev_i;
    unsigned int dis = 0;
    unsigned int len = 0;
    unsigned int skip;
    long j;

    unsigned int last_dis_arr[4];
    unsigned int last_len = 0;
    unsigned long last_dis_i = 0;

    unsigned int dis_used;

    int write_dis = 1;
    int write_len = 1;

    while (i < text_len - 3) {
        skip = search_fn(i, &dis, &len);
        prev_i = i;

        if (len < MIN_LEN) {
            len_lit_heads[len_lit_i++] = plain_text[i++];
        } else {
            for (j = 0; j < skip; ++j) {
                len_lit_heads[len_lit_i++] = plain_text[i++];
            }

            for (j = 1; j <= min(4, last_dis_i); ++j) {
                dis_used = last_dis_arr[(last_dis_i - j) & 3u];
                if (dis_used == dis) {
                    if (j == 1 && last_len == len) {
                        len_lit_heads[len_lit_i++] = BIG - 1;  // the last huff code
                        write_len = 0;
                    } else {
                        dis_heads[dis_head_i++] = SMALL - 5 + j;
                    }
                    write_dis = 0;
                    break;
                }
            }

            if (write_len) {
                len_head = write_length_bit(len, &bits, &bit_pos);
                len_lit_heads[len_lit_i++] = len_head;
            }

            if (write_dis) {
                dis_head = write_dis_bits(dis, &bits, &bit_pos);
                dis_heads[dis_head_i++] = dis_head;
            }

            last_dis_arr[last_dis_i & 3u] = dis;
            last_dis_i++;
            last_len = len;

            write_dis = 1;
            write_len = 1;

//            printf("lh: %u ", len_head);
//            printf("len %d dis %d lh %d, ", len, dis, len_head);

            flush_bits

            i += len;
        }

//        if (i > text_len) break;
        slider_fn(prev_i, i);
    }
    for (; i < text_len; ++i) {
        len_lit_heads[len_lit_i++] = plain_text[i];
    }
    len_lit_heads[len_lit_i++] = 256u;  // end sig
//    printf("finished\n");

    if (bit_pos > 0) {  // write the last bits
        bits <<= (8 - bit_pos);
        body_output[body_i++] = bits;
    }

    /// huffman part

    generate_freq_big(len_lit_heads, len_lit_i);
    if (dis_head_i > 0) generate_freq_small(dis_heads, dis_head_i);

    generate_huffman_table_big();
    if (dis_head_i > 0) generate_huffman_table_small();

    unsigned char *huf_out = malloc(len_lit_i + dis_head_i + body_i + 273 + 16);

    unsigned long result_len = 12;

    if (dis_head_i > 0) result_len += write_small_map(huf_out + result_len);
    result_len += write_big_map(huf_out + result_len);

//    unsigned long lh_begin = result_len;
//    printf("%ld\n", lh_begin);

    result_len += compress_big(len_lit_heads, len_lit_i, huf_out + result_len);
    unsigned long dis_head_st_index = result_len;
    if (dis_head_i > 0) result_len += compress_small(dis_heads, dis_head_i, huf_out + result_len);
    unsigned long body_st_index = result_len;

//    printf("%lu %lu\n", dis_head_st_index, body_st_index);

    int_to_bytes_32(huf_out, text_len);
    int_to_bytes_32(huf_out + 4, dis_head_st_index);  // record the compressed len_lit_head length
    int_to_bytes_32(huf_out + 8, body_st_index);

    memcpy(huf_out + result_len, body_output, body_i);
    result_len += body_i;

    *res_len = result_len;

//    printf("%d\n", huf_out[lh_begin]);

    free(len_lit_heads);
    free(dis_heads);
    free(body_output);

    return huf_out;
}

unsigned int empty_search(unsigned long index, unsigned int *dis_ptr, unsigned int *len_ptr) {
    return 0;
}

void empty_fill_slider(unsigned long prev_i, unsigned long i) {
}

void init_pool() {
    HASH_TABLE = create_adq_pool(HASH_SIZE);  // create the pool of ArrayDeque's
    ADQ_ARRAY_POOL = malloc(sizeof(unsigned long) * HASH_SIZE * ADQ_SIZE);
    for (int i = 0; i < HASH_SIZE; ++i) {
        ArrayDeque *adq = &HASH_TABLE[i];
        adq->tail = 0;
        adq->array = ADQ_ARRAY_POOL + i * ADQ_SIZE;
    }
}

unsigned char *compress(unsigned char *plain_text, unsigned long text_len, unsigned long *res_len, int level) {
    init_pool();

    if (level == 1) {  // huffman only
        return compress_content(plain_text, text_len, res_len, empty_search, empty_fill_slider);
    } else if (level == 2) {  // not using adq
        SIMPLE_HASH_TABLE = calloc(65536, sizeof(unsigned long));  // need to set all zero
        unsigned char *res = compress_content(plain_text, text_len, res_len, simple_search, simple_fill_slider);
        free(SIMPLE_HASH_TABLE);
        return res;
    } else if (level < 5) {
        return compress_content(plain_text, text_len, res_len, search_one_step, fill_slider);
    } else {
        return compress_content(plain_text, text_len, res_len, search_mul_steps, fill_slider);  // TODO
    }
}

void free_hashtable() {
    free(HASH_TABLE);
    free(ADQ_ARRAY_POOL);
}
