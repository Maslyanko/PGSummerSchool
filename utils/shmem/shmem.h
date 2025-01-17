#pragma once

#include <stdio.h>

#include "../utils/hashtable.h"

//Хэш-таблица, которая внутри себя хранит пары: имя общего региона - (имя общего региона, его начало, его размер).
extern HashTable* SHMEMS;
//Указатель на начало всего общего блока памяти, который состоит из именованных регионов.
extern void* MAIN_PTR;
//Размер общего блока памяти.
extern size_t SHMEMS_SIZE;
//Список запросов на создание именованного региона памяти.
extern List* SHMEM_REQUESTS;

//Функция, которая складывает запрос на выделение именнованного региона памяти в SHMEM_REQUESTS.
void requestShmem(char* name, size_t size);

//Функция, которая выделяет общий блок памяти, размер которого равен сумме всех запрашиваемых именованных регионов памяти, 
//далее проходится по SHMEM_REQUESTS и нарезает общий блок памяти на регионы.
void launchShmem();

//Возвращает указатель на начало именованного региона памяти.
void* getShmemPtr(char* name);

//Возвращает размер именованного региона памяти.
size_t getShmemSize(char* name);

//Освобождает всю информаци, для работы с разделяемой памятью, осовобождает общий блок памяти.
void finishShmem();