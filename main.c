#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "lib.h"
#include "lzz.h"
#include "lzz_d.h"

const char *USAGE = "Usage: lzz3.exe mode in_file [-o out_file]";

/*
 * Return code:  0 if normal finish, 1 if arguments error, 2 if compression error, 3 if decompression error
 */
int main(int argc, char **argv) {

    if (argc < 3) {
        printf("%s\n", USAGE);
        return 1;
    }

    char *mode = argv[1];
    char *inf = argv[2];
    char *ouf = NULL;

    int custom_ouf = 0;

    if (strlen(mode) != 2 || mode[0] != '-' || (mode[1] != 'c' && mode[1] != 'u')) {
        printf("Mode must be '-c' or '-u'");
        return 1;
    }

    if (argc > 4) {
        if (strcmp(argv[3], "-o") == 0) {
            ouf = argv[4];
            custom_ouf = 1;
        }
    }

    clock_t t0 = clock();

    unsigned long read;
    unsigned char *text = read_file(inf, &read);

//    clock_t read_finish = clock();

    if (mode[1] == 'c') {  // compression

        if (custom_ouf == 0) {  // No out file specified
            unsigned int inf_name_len = strlen(inf);
            ouf = malloc(inf_name_len + 5);
            memcpy(ouf, inf, inf_name_len);
            memcpy(ouf + inf_name_len, ".lzz", 4);
            ouf[inf_name_len + 4] = 0;
        }

        printf("File size: %ld\n", read);
        unsigned long res_len;
        unsigned char *cmp_text = compress(text, read, &res_len);
        if (write_file(ouf, cmp_text, res_len) != 0) {
            printf("IO error during compression\n");
            free(text);
            free(cmp_text);
            free_hashtable();
            if (!custom_ouf) {
                free(ouf);
            }
            return 2;
        }

        free(text);
        free(cmp_text);
        free_hashtable();

        clock_t t1 = clock();

        printf("Compression finished in %ld ms! Size after compression %ld, compression rate: %.2f%%\n",
               t1 - t0,
               res_len,
               (double) res_len / read * 100);

        if (ERROR_CODE != 0) {
            return 2;
        }
    } else {

        if (custom_ouf == 0) {  // No out file specified
            unsigned int inf_name_len = strlen(inf);
            if (end_with(inf, ".lzz")) {
                ouf = malloc(inf_name_len - 3);
                memcpy(ouf, inf, inf_name_len - 4);
                ouf[inf_name_len - 4] = 0;
            } else {
                ouf = malloc(inf_name_len + 5);
                memcpy(ouf, inf, inf_name_len);
                memcpy(ouf + inf_name_len, ".ori", 4);
                ouf[inf_name_len + 4] = 0;
            }
        }

        printf("Compressed file size: %ld\n", read);

        unsigned long ump_length;
        unsigned char *ump_text = uncompress(text, &ump_length);

//        clock_t unc_finish = clock();

        if (write_file(ouf, ump_text, ump_length) != 0) {
            printf("IO error during decompression\n");
            free(text);
            free(ump_text);
            if (!custom_ouf) {
                free(ouf);
            }
            return 3;
        }

        free(text);
        free(ump_text);

        clock_t t1 = clock();

//        printf("read time: %ld, unc time: %ld, write time: %ld\n", read_finish - t0, unc_finish - read_finish,
//                t1 - unc_finish);

        printf("Decompression finished in %ld ms! Original file size: %lu\n",
                t1 - t0,
                ump_length);
    }

    if (!custom_ouf) {
        free(ouf);
    }

    return 0;
}
