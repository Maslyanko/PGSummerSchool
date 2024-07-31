#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

#include "shmem.h"
#include "../utils/list.h"
#include "../alloc/alloc.h"

typedef struct ShmemRegion {
    char* name;
    void* ptr;
    size_t size;
} ShmemRegion;

HashTable* SHMEMS = NULL;
void* MAIN_PTR = NULL;
size_t SHMEMS_SIZE = 0;

List* SHMEM_REQUESTS = NULL;

void launchShmem() {
    if (SHMEMS_SIZE == 0) {
        return;
    }

    MAIN_PTR = mmap(NULL, SHMEMS_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0);

    ListNode* current = SHMEM_REQUESTS->head;
    void* currentPtr = MAIN_PTR;

    while (current) {
        ShmemRegion* region = (ShmemRegion*) current->data;

        region->ptr = currentPtr;
        insert(SHMEMS, region->name, region);

        currentPtr += region->size;
        current = current->next;
    }
}

void requestShmem(char* name, size_t size) {
    if (!MAIN_PTR) {
        SHMEM_REQUESTS = (List*) myMalloc(sizeof(List));
        initList(SHMEM_REQUESTS);
        SHMEMS = createHashTable(DEFAULT_HASHTABLE_SIZE, RESIZE_HASHTABLE_SCALE, HASHTABLE_BUCKET_CAPACITY);
    }

    ListNode* current = SHMEM_REQUESTS->head;

    while (current) {
        if (strcmp(((ShmemRegion*)current->data)->name, name) == 0) {
            break;
        }
        current = current->next;
    }

    ShmemRegion* region = NULL;

    if (current) {
        region = (ShmemRegion*) current->data;
    }

    if (region == NULL) {
        region = (ShmemRegion*) myMalloc(sizeof(ShmemRegion));
        region->name = (char*) myMalloc(strlen(name) + 1);
        strcpy(region->name, name);
        region->size = size;
        region->ptr = NULL;

        pushBack(SHMEM_REQUESTS, region);
        SHMEMS_SIZE += size;
    } else if (region->size != size) {
        SHMEMS_SIZE += size - region->size;
        region->size = size;
    }
}

void* getShmemPtr(char* name) {
    if (!MAIN_PTR) {
        return NULL;
    }

    ShmemRegion* region = (ShmemRegion*) getDataElement(SHMEMS, name);
    
    return region? region->ptr: NULL;
}

size_t getShmemSize(char* name) {
    if (!MAIN_PTR) {
        return -1;
    }

    ShmemRegion* region = (ShmemRegion*) getDataElement(SHMEMS, name);
    
    return region? region->size: -1;
}

void finishShmem() {
    if (!MAIN_PTR) {
        return;
    }

    munmap(MAIN_PTR, SHMEMS_SIZE);

    freeTable(SHMEMS);
    
    ListNode* current = SHMEM_REQUESTS->head;
    while (current) {
        ListNode* tmp = current;
        current = current->next;
        myFree(tmp);
    }

    myFree(SHMEM_REQUESTS);

    MAIN_PTR = NULL;
    SHMEMS_SIZE = 0;
    SHMEMS = NULL;
    SHMEM_REQUESTS = NULL;
}