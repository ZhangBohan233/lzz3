//
// Created by zbh on 2019/12/3.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lib.h"


unsigned char *read_file(char *file_name, unsigned long *length_ptr) {
    FILE *fp;
    int res = fopen_s(&fp, file_name, "rb");
    if (res != 0) {
        fclose(fp);
        perror("Open error when read");
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    int len = ftell(fp);
    *length_ptr = len;
    unsigned char *array = malloc(sizeof(unsigned char) * len);
    fseek(fp, 0, SEEK_SET);
    unsigned int read = fread(array, sizeof(unsigned char), len, fp);
    if (read != len) {
        fclose(fp);
        printf("Read error. Expected length %d, actual bytes %d\n", len, read);
        return NULL;
    }
    fclose(fp);
    return array;
}

int write_file(char *file_name, unsigned char *text, unsigned long length) {
    FILE *fp;
    int res = fopen_s(&fp, file_name, "wb");
    if (res != 0) {
        fclose(fp);
        perror("Open error when write");
        return 1;
    }

    fwrite(text, sizeof(unsigned char), length, fp);

    fclose(fp);

    return 0;
}

void int_to_bytes_16(unsigned char *b, unsigned long i) {
    *b = (i >> 8u);
    *(b + 1) = i;
}

void int_to_bytes_32(unsigned char *b, unsigned long i) {
    *b = (i >> 24u);
    *(b + 1) = (i >> 16u);
    *(b + 2) = (i >> 8u);
    *(b + 3) = i;
}

unsigned long bytes_to_int_32(const unsigned char *b) {
    return ((unsigned long) *b << 24u) |
           ((unsigned long) *(b + 1) << 16u) |
           ((unsigned long) *(b + 2) << 8u) |
           (unsigned long) *(b + 3);
}

/*
 * Returns 0 if <s> does not ends with <pattern>, 1 otherwise
 */
int end_with(const char *s, const char *pattern) {
    unsigned int s_len = strlen(s);
    unsigned int p_len = strlen(pattern);
    if (p_len > s_len) return 0;

    unsigned int p_start = s_len - p_len;

    for (int i = 0; i < p_len; ++i) {
        if (s[p_start + i] != pattern[i]) return 0;
    }
    return 1;
}
