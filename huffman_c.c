//
// Created by zbh on 2020/1/14.
//

#include <stdio.h>
#include <corecrt_search.h>
#include <stdlib.h>
#include "huffman_c.h"
#include "lib.h"

#define flush_bits while (bit_pos >= 8) {   \
    bit_pos -= 8;                           \
    temp = (bits >> bit_pos) & 0xffu;       \
    out[res_index++] = temp;                \
}


unsigned int FREQ_SMALL[16];
unsigned int FREQ_BIG[273];
unsigned int CODE_LENGTH_SMALL[16];
unsigned int CODE_LENGTH_BIG[273];
unsigned int CODE_SMALL[16];
unsigned int CODE_BIG[273];

void print_binary(unsigned int value, unsigned int bit_len) {
    for (int i = 0; i < bit_len; i++) {
        printf("%u", (value >> (bit_len - i - 1)) & 0x01u);
    }
}

void print_array(const unsigned int *array, unsigned int size) {
    printf("[");
    for (int i = 0; i < size; ++i) {
        printf("%u, ", array[i]);
    }
    printf("]\n");
}

void print_canonical_code(const unsigned int *codes, const unsigned int *length_map, unsigned int size) {
    printf("[");
    for (int i = 0; i < size; ++i) {
        unsigned int value = codes[i];
        unsigned int cl = length_map[i];
        print_binary(value, cl);
        printf(", ");
    }
    printf("]\n");
}

void generate_freq_big(const unsigned short *text, unsigned long length) {
    for (long i = 0; i < length; ++i) {
        FREQ_BIG[text[i]] += 1;
    }
    FREQ_BIG[256] = 1;  // end sig
}

void generate_freq_small(const unsigned char *text, unsigned long length) {
    for (long i = 0; i < length; ++i) {
        FREQ_SMALL[text[i]] += 1;
    }
}

int cmp_node(const void *p1, const void *p2) {
    HufNode **left = (HufNode **) p1;
    HufNode **right = (HufNode **) p2;
//    printf("left: %u, right: %u\n", (*left)->freq, (*right)->freq);
    if ((*left)->freq < (*right)->freq) return 1;
    else if ((*left)->freq > (*right)->freq) return -1;
    else return 0;
}

int cmp_tuple(const void *p1, const void *p2) {
    HufTuple *left = (HufTuple *) p1;
    HufTuple *right = (HufTuple *) p2;
//    printf("left: %u, right: %u\n", left->length, right->length);
    if (left->length < right->length) {
        return -1;
    } else if (left->length > right->length) {
        return 1;
    } else {
        if (left->value < right->value) {
            return -1;
        } else {  // value could not be equal
            return 1;
        }
    }
}

HufNode *generate_root(const unsigned int *freq_table, unsigned int table_size) {
    HufNode *list[273];
    unsigned int size = 0;
    for (unsigned int i = 0; i < table_size; ++i) {
        if (freq_table[i] > 0) {
            HufNode *node = malloc(sizeof(HufNode));
            node->value = i;
            node->freq = freq_table[i];
            node->left = NULL;
            node->right = NULL;
            list[size++] = node;
        }
    }
    while (size > 1) {
        qsort(list, size, sizeof(HufNode *), cmp_node);
        HufNode *right = list[--size];
        HufNode *left = list[--size];

        HufNode *new = malloc(sizeof(HufNode));
        new->left = left;
        new->right = right;
        new->freq = left->freq + right->freq;
        list[size++] = new;
    }
    return list[0];
}

void free_tree(HufNode *node) {
    if (node != NULL) {
        free_tree(node->left);
        free_tree(node->right);
        free(node);
    }
}

void generate_code_length(unsigned int *length_map, HufNode *node, unsigned int length) {
    if (node != NULL) {
        if (node->left == NULL && node->right == NULL) {
            // is leaf
            length_map[node->value] = length;
        } else {
            generate_code_length(length_map, node->left, length + 1);
            generate_code_length(length_map, node->right, length + 1);
        }
    }
}

void generate_canonical_code(unsigned int *codes, const unsigned int *length_map, unsigned int length) {
    HufTuple list[273];
    unsigned int size = 0;
    for (unsigned int i = 0; i < length; ++i) {
        if (length_map[i] > 0) {
            HufTuple ht;
            ht.value = i;
            ht.length = length_map[i];
            list[size++] = ht;
        }
    }
    qsort(list, size, sizeof(HufTuple), cmp_tuple);
    codes[list[0].value] = 0;
    unsigned int code = 0;
    for (unsigned int i = 1; i < size; ++i) {
        code = (code + 1) << (list[i].length - list[i - 1].length);
        codes[list[i].value] = code;
    }
}

void generate_huffman_table_big() {
//    print_array(FREQ_BIG, 273);
    HufNode *root = generate_root(FREQ_BIG, 273);

    generate_code_length(CODE_LENGTH_BIG, root, 0);
//    print_array(CODE_LENGTH_BIG, 273);

    generate_canonical_code(CODE_BIG, CODE_LENGTH_BIG, 273);
//    print_array(CODE_BIG, 273);
//    print_canonical_code(CODE_BIG, CODE_LENGTH_BIG, 273);

    free_tree(root);
}

void generate_huffman_table_small() {
    HufNode *root = generate_root(FREQ_SMALL, 8);
    generate_code_length(CODE_LENGTH_SMALL, root, 0);
    generate_canonical_code(CODE_SMALL, CODE_LENGTH_SMALL, 8);
//    print_array(FREQ_SMALL, 8);
//    print_array(CODE_LENGTH_SMALL, 8);
//    print_array(CODE_SMALL, 8);
//    print_canonical_code(CODE_SMALL, CODE_LENGTH_SMALL, 8);

    free_tree(root);
}

unsigned long write_small_map(unsigned char *out) {
    unsigned int res = 0;
    unsigned int pos = 0;
    unsigned long rp = 0;
    for (int i = 0; i < 16; ++i) {
        unsigned int len = CODE_LENGTH_SMALL[i];
        res <<= 5u;
        res |= len;
        pos += 5;
        if (pos >= 8) {
            out[rp] = res;
            pos -= 8;
            rp += 1;
        }
    }
    return rp;
}

unsigned long write_big_map(unsigned char *out) {
    unsigned int rle[273];
    unsigned int rle_len = 0;
    unsigned int count = 0;
    for (int i = 0; i < 273; ++i) {
        unsigned int len = CODE_LENGTH_BIG[i];
        if (len == 0) {
            count++;
        } else {
            if (count == 0) {
                rle[rle_len++] = len;
            } else if (count == 1) {
                rle[rle_len++] = 0;
            } else if (count < 32) {
                rle[rle_len++] = 30;
                rle[rle_len++] = count;
            } else {
                rle[rle_len++] = 31;
                rle[rle_len++] = count >> 5u;
                rle[rle_len++] = count & 0x1fu;
            }
            count = 0;
        }
    }

    unsigned int res = 0;
    unsigned int pos = 0;
    unsigned long rp = 0;

    for (int i = 0; i < rle_len; ++i) {
        unsigned int len = rle[i];
        res <<= 5u;
        res |= len;
        pos += 5;
        if (pos >= 8) {
            out[rp] = res;
            pos -= 8;
            rp += 1;
        }
    }
    if (pos > 0) {  // write the last bits
        res <<= (8 - pos);
        out[rp++] = res;
    }

    return rp;
}

unsigned long compress_small(const unsigned char *text, unsigned long text_len,
                             unsigned char *out) {
    unsigned long res_index = 0;
    unsigned long bits = 0;
    unsigned int bit_pos = 0;
    unsigned char c;
    unsigned int code;
    unsigned int length;
    unsigned int temp;

//    int_to_bytes_32(out, text_len);  // record the orig text len

    for (long i = 0; i < text_len; ++i) {
        c = text[i];
        code = CODE_SMALL[c];
        length = CODE_LENGTH_SMALL[c];
        bits <<= length;
        bits |= code;
        bit_pos += length;

        while (bit_pos >= 8) {
            bit_pos -= 8;
            temp = (bits >> bit_pos) & 0xffu;
            out[res_index++] = temp;
        }
    }
    if (bit_pos > 0) {  // write the last bits
        bits <<= (8 - bit_pos);
        out[res_index++] = bits;
    }

    return res_index;
}

unsigned long compress_big(const unsigned short *text, unsigned long text_len,
                           unsigned char *out) {
    unsigned long res_index = 0;
    unsigned long bits = 0;
    unsigned int bit_pos = 0;
    unsigned short c;
    unsigned int code;
    unsigned int length;
    unsigned int temp;

    for (long i = 0; i < text_len; ++i) {
        c = text[i];
        code = CODE_BIG[c];
        length = CODE_LENGTH_BIG[c];
        bits <<= length;
        bits |= code;
        bit_pos += length;

        flush_bits
    }

    unsigned int end_sig = CODE_BIG[256];
    unsigned int end_sig_len = CODE_LENGTH_BIG[256];

    bits <<= end_sig_len;
    bits |= end_sig;
    bit_pos += end_sig_len;

    flush_bits

    if (bit_pos > 0) {  // write the last bits
        bits <<= (8 - bit_pos);
        out[res_index++] = bits;
    }

    return res_index;
}
