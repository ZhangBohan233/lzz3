//
// Created by zbh on 2020/1/14.
//

#ifndef LZZ3_HUFFMAN_C_H
#define LZZ3_HUFFMAN_C_H

typedef struct HufNode {
    unsigned short value;
    unsigned int freq;
    struct HufNode *left;
    struct HufNode *right;
} HufNode;

void generate_freq_big(const unsigned short *text, unsigned long length);

void generate_freq_small(const unsigned char *text, unsigned long length);

void generate_huffman_table_big();

void generate_huffman_table_small();

unsigned long write_small_map(unsigned char *out);

unsigned long write_big_map(unsigned char *out);

unsigned long compress_small(const unsigned char *text, unsigned long text_len,
        unsigned char *out);

unsigned long compress_big(const unsigned short *text, unsigned long text_len,
        unsigned char *out);

void print_array(const unsigned int *array, unsigned int size);

void generate_canonical_code(unsigned int *codes, const unsigned int *length_map, unsigned int length);

void print_canonical_code(const unsigned int *codes, const unsigned int *length_map, unsigned int size);

void print_binary(unsigned int value, unsigned int bit_len);

#endif //LZZ3_HUFFMAN_C_H
