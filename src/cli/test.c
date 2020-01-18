//#include <stdio.h>
//#include <time.h>
//#include <stdlib.h>
//
//#include "lib.h"
//#include "lzz.h"
//#include "lzz_d.h"
//
//int main(int argc, char **argv) {
//
//    char *inf = argv[1];
//    char *ouf = argv[2];
//
//    clock_t t0 = clock();
//
//    unsigned long read;
//    unsigned char *text = read_file(inf, &read);
//
//    printf("File size: %ld\n", read);
//
//    unsigned long res_len;
//    unsigned char *cmp_text = compress(text, read, &res_len);
//
//    printf("Size after compression %ld, compression rate: %.2f%%\n", res_len, (double) res_len / read * 100);
//
//    if (ERROR_CODE != 0) return 1;
//
//    write_file(ouf, cmp_text, res_len);
//
//    clock_t t1 = clock();
//    printf("Total compress time: %ld\n", t1 - t0);
//
//    free(text);
//    free(cmp_text);
//    free_hashtable();
//
//
//    clock_t t2 = clock();
//
//    unsigned long read2;
//    unsigned char *cmp_text2 = read_file(ouf, &read2);
//
//    unsigned long ump_length;
//    unsigned char *ump_text = uncompress(cmp_text2, &ump_length);
//
//    char *dec_name = "../uncompress.zip";
//
//    write_file(dec_name, ump_text, ump_length);
//
//    printf("Original size: %lu\n", ump_length);
//
//    clock_t t3 = clock();
//    printf("Total uncompress time: %ld\n", t3 - t2);
//
//    free(ump_text);
//
//    return 0;
//}
//
//
