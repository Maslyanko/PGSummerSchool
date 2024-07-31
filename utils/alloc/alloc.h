#pragma once

#include "stdio.h"

void* myMalloc(size_t size);

void* myCalloc(size_t number, size_t size);

void* myRealloc(void* ptr, size_t newSize);

void myFree(void* ptr);