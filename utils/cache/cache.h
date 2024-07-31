#pragma once

#include <stddef.h>
#include <semaphore.h>

extern sem_t* CACHE_SEMAPHORE;

//Функция инициализации кэша.
void initCache(size_t capacity);

//Функция, возвращающая УКАЗАТЕЛЬ на данные, лежащие в кэше.
char* getDataFromCache(char* key);

//ФУнкция, сохраняющая УКАЗАТЕЛЬ на данные и УКАЗАТЕЛЬ на ключ. Ничего не копируется.
void insertDataIntoCache(char* key, char* data);

//Вычищает все данные, лежащие в кэше.
void freeCache();

//Завершает работу с кэшом.
void finishCache();