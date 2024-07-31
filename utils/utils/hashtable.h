#pragma once

#include <stdio.h>

#include "list.h"

//Стандартный размер хэш-таблицы
#define DEFAULT_HASHTABLE_SIZE 1000
//Величина, которая задаёт во сколько раз увеличится размер хэш-таблицы, при условии частых коллизий.
#define RESIZE_HASHTABLE_SCALE 1.26
//По сути задаёт условие увеличение размера хэш-таблицы. Задаёт размер листа, в который попадают элементы с одним хэшом.
//Если размер листа достиг этого значения, значит надо увеличить размер хэш-таблицы.
//Размер хэш-таблицы может увеличиться при вставке, если сработает это суловие.
#define HASHTABLE_BUCKET_CAPACITY 10

//Пара: ключ - значение. Значением является указатель на данные, а ключом указатель на строку.
//НИЧЕГО НЕ КОПИРУЕТСЯ при вставке, просто сохраняется указатель.
typedef struct HashTableEntry {
    char* key;
    void* data;
} HashTableEntry;

//Сама хэш-таблица. Особо без комментариев.
typedef struct HashTable {
    size_t size;              //См. DEFAULT_HASHTABLE_SIZE
    double resizeScale;       //См. RESIZE_HASHTABLE_SCALE
    size_t scalar;            //Число, необходимое в хэш-функции.
    size_t bucketCapacity;    //См. HASHTABLE_BUCKET_CAPACITY
    size_t count;             //Кол-во элементов, лежащих внутри таблицы
    List* buckets;            //Списки, в которые попадают элементы при коллизии.
} HashTable;

//Выделяет память под таблицы и инициализирует её.
HashTable* createHashTable(size_t size, double resizeScale, size_t bucketCapacity);

//Вставляет новый элемент. Опять же, сохраняет УКАЗАТЕЛИ на данные и ключ, НЕ КОПИРУЕТ.
void insert(HashTable* table, char* key, void* data);

//Возвращает элемент из таблицы по ключу. Если такого нет, то возвращает NULL.
void* getDataElement(HashTable* table, char* key);

//Удаляет элемент из таблицы по ключу. Очищает указатели ключа и данных.
void removeElement(HashTable* table, char* key);

//Возвращает все пары, лежащие в таблице.
HashTableEntry* getAllElements(HashTable* table);

//Удаляет все элементы из таблицы.
void cleanTable(HashTable* table);

//Удаляет все элементы из таблицы и очищает саму таблицу.
void freeTable(HashTable* table);