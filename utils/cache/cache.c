#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#include "../alloc/alloc.h"
#include "../utils/hashtable.h"
#include "../utils/list.h"
#include "../logger/logger.h"

typedef struct CacheTableEntry {
    char* key;
    char* data;
    ListNode* listNode;
} CacheTableEntry;

HashTable* CACHE_TABLE = NULL;
List* CACHE_LIST = NULL;
int CACHE_CAPACITY = 100;

void initCache(size_t capacity) {
    if (CACHE_TABLE) {
        return;
    }

    if (capacity <= 0) {
        capacity = CACHE_CAPACITY;
    } else {
        CACHE_CAPACITY = capacity;
    }

    CACHE_TABLE = createHashTable(capacity * 100, RESIZE_HASHTABLE_SCALE, HASHTABLE_BUCKET_CAPACITY);

    CACHE_LIST = (List*) myMalloc(sizeof(List));
    initList(CACHE_LIST);
}


char* getDataFromCache(char* key) {
    if (!CACHE_TABLE) {
        return NULL;
    }

    CacheTableEntry* cacheRow = (CacheTableEntry*) getDataElement(CACHE_TABLE, key);

    if (!cacheRow) {
        return NULL;
    }

    if (cacheRow->listNode == CACHE_LIST->head) {
        return cacheRow->data;
    } else if (cacheRow->listNode == CACHE_LIST->tail) {
        CACHE_LIST->tail = cacheRow->listNode->prev;
        CACHE_LIST->tail->next = NULL;
    } else {
        cacheRow->listNode->prev->next = cacheRow->listNode->next;
        cacheRow->listNode->next->prev = cacheRow->listNode->prev;
    }

    cacheRow->listNode->prev = NULL;
    cacheRow->listNode->next = CACHE_LIST->head;

    CACHE_LIST->head->prev = cacheRow->listNode;
    CACHE_LIST->head = cacheRow->listNode;

    return cacheRow->data;
}

void insertDataIntoCache(char* key, char* data) {
    if (!CACHE_TABLE) {
        initCache(CACHE_CAPACITY);
    }

    if (getDataElement(CACHE_TABLE, key)) {
        return;
    }

    CacheTableEntry* cacheEntry = (CacheTableEntry*) myMalloc(sizeof(CacheTableEntry));
    cacheEntry->data = data;
    cacheEntry->key = key;
    
    if (CACHE_TABLE->count == CACHE_CAPACITY) {
        CacheTableEntry* pulledRow = (CacheTableEntry*) popBack(CACHE_LIST);
        myFree(pulledRow->data);
        removeElement(CACHE_TABLE, pulledRow->key);
    }

    pushFront(CACHE_LIST, cacheEntry);
    cacheEntry->listNode = CACHE_LIST->head;
    insert(CACHE_TABLE, cacheEntry->key, cacheEntry);
}

void freeCache() {
    if (!CACHE_TABLE) {
        return;
    }

    ListNode* current = CACHE_LIST->head;

    while (current) {
        CacheTableEntry* entry = (CacheTableEntry*) current->data;
        myFree(entry->key);
        myFree(entry->data);
        myFree(entry);
        current = current->next;
    }

    freeTable(CACHE_TABLE);
    
    CACHE_TABLE->count = 0;
}

void finishCache() {
    if (!CACHE_TABLE) {
        return;
    }
    freeCache();
    myFree(CACHE_TABLE);
    CACHE_TABLE = NULL;
}