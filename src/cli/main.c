#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "../core/lib.h"
#include "../core/array_deque.h"
#include "../core/lzz.h"
#include "../core/lzz_d.h"

const char *USAGE =
        "Usage: lzz3.exe <mode> <in_file> [-l level] [-o out_file]"
        "  mode:"
        "    -c    compress"
        "    -u    uncompress";

void set_level(int level) {
    switch (level) {
        case 1:  // huffman only
            break;
        case 2:  // not using adq
            set_window(32768, 32);
            break;
        case 3:
            ADQ_SIZE = 4;
            ADQ_AND = 3;
            set_window(32768, 32);
            break;
        case 4:
        case 5:  // begins to non-greedy search
            ADQ_SIZE = 16;
            ADQ_AND = 15;
            set_window(32768, 128);
            break;
        case 6:
            ADQ_SIZE = 32;
            ADQ_AND = 31;
            set_window(65536, 128);
            break;
        case 7:
            ADQ_SIZE = 64;
            ADQ_AND = 63;
            set_window(65536, 128);
            break;
        case 8:
            ADQ_SIZE = 128;
            ADQ_AND = 127;
            set_window(252144, 255);
            break;
        case 9:
            ADQ_SIZE = 256;
            ADQ_AND = 255;
            set_window(1048576, 255);
            break;
        default:
            printf("Invalid compression level %d\n", level);
            exit(1);
    }
}

int main_use(int argc, char **argv) {
    char *mode = argv[1];
    char *inf = argv[2];
    char *ouf = NULL;

    int custom_ouf = 0;

    if (strlen(mode) != 2 || mode[0] != '-' || (mode[1] != 'c' && mode[1] != 'u' && mode[1] != 't')) {
        printf("Mode must be '-c' or '-u' or '-t'");
        return 1;
    }

    int level = 0;
    if (argc > 4) {
        for (int i = 3; i < argc; ++i) {
            if (strcmp(argv[i], "-o") == 0) {
                ouf = argv[i + 1];
                custom_ouf = 1;
                i += 1;
            } else if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--level") == 0) {
                level = strtol(argv[i + 1], NULL, 10);
                i += 1;
            }
        }
    }

    clock_t t0 = clock();

    unsigned long read;
    unsigned char *text = read_file(inf, &read);

//    clock_t read_finish = clock();

    if (mode[1] == 'c') {  // compression
        if (level == 0) level = 5;  // default level
        set_level(level);
        if (custom_ouf == 0) {  // No out file specified
            unsigned int inf_name_len = strlen(inf);
            ouf = malloc(inf_name_len + 5);
            memcpy(ouf, inf, inf_name_len);
            memcpy(ouf + inf_name_len, ".lzz", 4);
            ouf[inf_name_len + 4] = 0;
        }

        printf("File size: %ld\n", read);
        unsigned long res_len;
        unsigned char *cmp_text = compress(text, read, &res_len, level);
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
    } else if (mode[1] == 'u') {
        if (level != 0) printf("Option '-l', '--level' only useful in mode '-c'.\n");
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
    } else {  // test mode

    }

    if (!custom_ouf) {
        free(ouf);
    }

    return 0;
}

void test() {
    unsigned int dis = 2;
    unsigned long bits = 0;
    unsigned int bit_pos = 0;

    unsigned int a = write_dis_bits(dis, &bits, &bit_pos);

    printf("%u %lu %u\n", a, bits, bit_pos);
}

/*
 * Return code:  0 if normal finish, 1 if arguments error, 2 if compression error, 3 if decompression error
 */
int main(int argc, char **argv) {

    if (argc < 3) {
        printf("%s\n", USAGE);
        return 1;
    }

//    test();
//
    int result = main_use(argc, argv);

    return result;
}
