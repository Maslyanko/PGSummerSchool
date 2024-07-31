#include <stdlib.h>

#include "alloc.h"

void* myMalloc(size_t size) {
    return malloc(size);
}

void* myCalloc(size_t number, size_t size) {
    return calloc(number, size);
}

void* myRealloc(void* ptr, size_t newSize) {
    return realloc(ptr, newSize);
}

void myFree(void* ptr) {
    free(ptr);
}