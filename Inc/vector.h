/**
*
*  Provide implementation of dynamic vector in C
*
*  This implementation file provide functions for implementing dynamic vector.
*
*@source: https://eddmann.com/posts/implementing-a-dynamic-vector-array-in-c/
*
*/

#ifndef VECTOR_H
#define VECTOR_H

#define VECTOR_INIT_CAPACITY 4

typedef struct vector {
    void **items;
    int capacity;
    int total;
} vector;

void vector_init(vector *);
int vector_total(vector *);
void vector_add(vector *, void *);
void vector_set(vector *, int, void *);
void *vector_get(vector *, int);
void vector_delete(vector *, int);
void vector_free(vector *);

#endif
