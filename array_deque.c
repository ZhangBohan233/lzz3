//
// Created by zbh on 2019/12/26.
//

#include <stdlib.h>
#include <stdio.h>
#include "array_deque.h"

ArrayDeque *create_adq() {
    ArrayDeque *adq = malloc(sizeof(ArrayDeque));
    adq->tail = 0;
    return adq;
}

void adq_add_last(ArrayDeque *adq, unsigned long value) {
    adq->array[(adq->tail) & 15u] = value;
    adq->tail++;
}

unsigned int adq_begin_pos(ArrayDeque *adq) {
    return adq->tail >= 16 ? adq->tail - 16 : 0;
}

void print_adq(ArrayDeque *adq) {
    unsigned int abp = adq_begin_pos(adq);
    printf("Adq %d [", adq->tail);
    for (unsigned int i = abp; i < adq->tail; ++i) {
        printf("%lu ", adq->array[i & 15u]);
    }
    printf("]\n");
}
