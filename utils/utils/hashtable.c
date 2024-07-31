#include <time.h>
#include <string.h>
#include <stdlib.h>

#include "hashtable.h"
#include "../alloc/alloc.h"
#include "list.h"

static size_t hashFunction(HashTable* table, const char* key) {
    size_t hash = 0;

    for (size_t i = 0; key[i]; i++) {
        hash += (size_t)key[i] * (i + 1);
    }

    return (hash * table->scalar) % table->size;
}

static void resizeTable(HashTable* table) {
    HashTable* newTable = createHashTable(table->size * table->resizeScale, table->resizeScale, table->bucketCapacity);

    for (size_t i = 0; i < table->size; i++) {
        ListNode* current = table->buckets[i].head;

        while (current) {
            HashTableEntry* entry = (HashTableEntry*) current->data;
            insert(newTable, entry->key, entry->data);
            current = current->next;
        }
    }

    freeTable(table);
    *table = *newTable;
}

HashTable* createHashTable(size_t size, double resizeScale, size_t bucketCapacity) {
    HashTable* table = (HashTable*) myMalloc(sizeof(HashTable));

    if (!table) {
        return NULL;
    }

    srand(time(NULL));

    table->size = size;
    table->resizeScale = resizeScale;
    table->bucketCapacity = bucketCapacity;
    table->scalar = rand() % size / resizeScale;
    table->count = 0;

    table->buckets = (List*) myCalloc(size, sizeof(List));

    if (!table->buckets) {
        myFree(table);
        return NULL;
    }

    for (size_t i = 0; i < size; i++) {
        initList(&table->buckets[i]);
    }

    return table;
}

void insert(HashTable* table, char* key, void* data) {
    size_t index = hashFunction(table, key);
    ListNode* current = table->buckets[index].head;

    int bucketCount = 0;

    while (current && current->next) {
        current = current->next;
        bucketCount++;
    }

    if (bucketCount == table->bucketCapacity) {
        resizeTable(table);
        index = hashFunction(table, key);
    }
    
    ListNode* newNode = (ListNode*) myMalloc(sizeof(ListNode));
    HashTableEntry* entry = (HashTableEntry*) myMalloc(sizeof(HashTableEntry));

    newNode->data = entry;
    entry->key = key;
    entry->data = data;
    newNode->next = NULL;

    if (current == NULL) {
        table->buckets[index].head = newNode;
        newNode->prev = NULL;
    } else {
        current->next = newNode;
        newNode->prev = current;
    }

    table->count++;
}

void* getDataElement(HashTable* table, char* key) {
    size_t index = hashFunction(table, key);
    ListNode* current = table->buckets[index].head;

    while (current) {
        HashTableEntry* entry = (HashTableEntry*) current->data;

        if (strcmp(entry->key, key) == 0) {
            return entry->data;
        }

        current = current->next;
    }

    return NULL;
}

void removeElement(HashTable* table, char* key) {
    size_t index = hashFunction(table, key);
    ListNode* current = table->buckets[index].head;

    while (current) {
        HashTableEntry* entry = (HashTableEntry*) current->data;
        
        if (strcmp(entry->key, key) == 0) {
            if (current->prev) {
                current->prev->next = current->next;
            } else {
                table->buckets[index].head = current->next;
            }

            if (current->next) {
                current->next->prev = current->prev;
            }

            myFree(entry->key);
            myFree(entry->data);
            myFree(entry);
            myFree(current);
            table->count--;
            return;
        }

        current = current->next;
    }
}

HashTableEntry* getAllElements(HashTable* table) {
    HashTableEntry* entries = (HashTableEntry*) myMalloc(table->count * sizeof(HashTableEntry));
    size_t count = 0;

    for (size_t i = 0; i < table->size; i++) {
        ListNode* current = table->buckets[i].head;

        while (current) {
            HashTableEntry* entry = (HashTableEntry*) current->data;
            entries[count] = *entry;
            count++;
            current = current->next;
        }
    }

    return entries;
}

void cleanTable(HashTable* table) {
    for (size_t i = 0; i < table->size; i++) {
        ListNode* current = table->buckets[i].head;

        while (current) {
            ListNode* temp = current;
            current = current->next;
            myFree(temp->data);
            myFree(temp);
        }

        initList(&table->buckets[i]);
    }

    table->count = 0;
}

void freeTable(HashTable* table) {
    cleanTable(table);
    myFree(table);
}