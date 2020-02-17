//
// Created by zbh on 2019/12/26.
//

#ifndef LZZ3_ARRAY_DEQUE_H
#define LZZ3_ARRAY_DEQUE_H

extern unsigned int ADQ_SIZE;

/*
 * The array deque index and-er
 */
extern unsigned int ADQ_AND;

typedef struct {
    unsigned int tail;
    unsigned long *array;
} ArrayDeque;

ArrayDeque *create_adq();

void adq_add_last(ArrayDeque *adq, unsigned long value);

unsigned int adq_begin_pos(ArrayDeque *adq);

void print_adq(ArrayDeque *adq);

void free_adq(ArrayDeque *adq);

//void remove_first(ArrayDeque *adq, unsigned long value);

#endif //LZZ3_ARRAY_DEQUE_H
