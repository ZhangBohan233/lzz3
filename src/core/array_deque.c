//
// Created by zbh on 2019/12/26.
//

#include <stdlib.h>
#include <stdio.h>
#include "array_deque.h"

unsigned int ADQ_SIZE = 32;
unsigned int ADQ_AND = 0x1f;

ArrayDeque *create_adq() {
    ArrayDeque *adq = malloc(sizeof(ArrayDeque));
    adq->tail = 0;
    adq->array = malloc(sizeof(unsigned long) * ADQ_SIZE);
    return adq;
}

void free_adq(ArrayDeque *adq) {
    if (adq != NULL) {
        free(adq->array);
        free(adq);
    }
}

void adq_add_last(ArrayDeque *adq, unsigned long value) {
    adq->array[(adq->tail) & ADQ_AND] = value;
    adq->tail++;
}

unsigned int adq_begin_pos(ArrayDeque *adq) {
    return adq->tail >= ADQ_SIZE ? adq->tail - ADQ_SIZE : 0;
}

void print_adq(ArrayDeque *adq) {
    unsigned int abp = adq_begin_pos(adq);
    printf("Adq %d [", adq->tail);
    for (unsigned int i = abp; i < adq->tail; ++i) {
        printf("%lu ", adq->array[i & ADQ_AND]);
    }
    printf("]\n");
}
