#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define MEM_UNIT (char) 8
#define LOG2_NUM_MAPPINGS (char) 15
#define METADATA_OFFSET (sizeof(long))

typedef struct header {
    unsigned long size:48;
    unsigned long mapping:LOG2_NUM_MAPPINGS;
    unsigned long free:1;
    struct header* next;
    struct header* prev;
} header;

typedef struct tail {
    unsigned long size;
} tail;

#define MIN_ALLOC (sizeof (header) + sizeof(tail))
#define MMAP_UNIT ((1 << 5) * sysconf(_SC_PAGESIZE))
#define NUM_BUCKETS (unsigned char) 166
#define FULL_BLOCK_SIZE(x) x + METADATA_OFFSET + sizeof(tail)

static header BUCKETS[NUM_BUCKETS];
static bool INITIALIZED = false;
static unsigned short MAPPING_INDEX = 0;
static unsigned long MAPPINGS[1 << LOG2_NUM_MAPPINGS][2];

static void initializeBuckets(void);
static size_t aligned(size_t size);
static header* getBlockFromBuckets(size_t size);
static header* getBlockFromOS(size_t size);
static header* getBlockFromPtr(void* ptr);
static void insertIntoBuckets(header* inserted);
static void join(header* lower, header* higher);
static header* trimBlock(header* block, size_t size);


void* myMalloc(size_t size) {
    size_t alignedSize = aligned(size);

    if (size == 0 || alignedSize < size) {
        errno = EINVAL;
        return NULL;
    }

    if (!INITIALIZED) {
        initializeBuckets();
    }

    header* header = getBlockFromBuckets(alignedSize);

    if (header == NULL) {
        header = getBlockFromOS(alignedSize);
    }

    if (header == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    header->free = false;

    return (void*) ((unsigned long) header + METADATA_OFFSET);
}

void myFree(void* ptr) {
    if (!ptr) {
        return;
    }

    header* block = getBlockFromPtr(ptr);
    insertIntoBuckets(block);

    unsigned long nextBlock = (unsigned long) block + block->size;
    bool nextBlockIsInOurSpace = nextBlock < MAPPINGS[block->mapping][0];
    if (nextBlockIsInOurSpace && (bool) ((header*) nextBlock)->free) {
        join(block, (header*) nextBlock);
    }

    bool blockIsFirst = (unsigned long) block == MAPPINGS[block->mapping][0];
    if (blockIsFirst) {
        return;
    }

    header* prevBlock = (header*) ((unsigned long) block - ((tail*) block - 1)->size);

    if (prevBlock->free) {
        join(prevBlock, block);
    }
}

void* myCalloc(size_t number, size_t size) {
    if (number == 0 || size > (size_t) - 1 / number) {
        errno = EINVAL;
        return NULL;
    }

    size_t totalSize = number * size;

    void* ptr = myMalloc(totalSize);
    if (!ptr) {
        return NULL;
    }

    memset(ptr, 0, totalSize);
    return ptr;
}

void* myRealloc(void* ptr, size_t size) {
    if (!ptr || !size) {
        myFree(ptr);
        return myMalloc(size);
    }

    header* block = getBlockFromPtr(ptr);
    unsigned int oldSize = FULL_BLOCK_SIZE(block->size);

    if (size <= oldSize) {
        return (void*) ((unsigned long) trimBlock(block, aligned(size)) + METADATA_OFFSET);
    }

    void* newPtr = myMalloc(size);
    if (newPtr) {
        memcpy(newPtr, block, oldSize);
    }

    myFree(ptr);
    return newPtr;
}


size_t roundUpPower(size_t naumber, size_t power) {
    return ((naumber + (power - 1)) & ~(power - 1));
}

void updateSize(header* block, size_t size) {
    block->size = ((tail*) ((unsigned long) block + size) - 1)->size = size;
}

void initializeBuckets(void) {
    static const header dummyHeader = {.size = 0};
    for (unsigned int i = 0; i < NUM_BUCKETS; ++i) {
        BUCKETS[i] = dummyHeader;
        BUCKETS[i].next = BUCKETS[i].prev = &BUCKETS[i];
    }
    INITIALIZED = true;
}

size_t aligned(size_t size) {
    size = (FULL_BLOCK_SIZE(size) + (MEM_UNIT - 1)) & ~(MEM_UNIT - 1);
    return size >= MIN_ALLOC ? size: MIN_ALLOC;
}

unsigned int bucketIndexFromSize(size_t size) {
    static const unsigned int index1024 = 1024 / MEM_UNIT;
    if (size < 1024) {
        return size / MEM_UNIT;
    }

    unsigned int log = 0;

    for (unsigned int log = 10; size >> (log + 1) > 0; ++log);
    return index1024 + (log - 10);
}

void removeFromBucket(header* block) {
    block->next->prev = block->prev;
    block->prev->next = block->next;
}

header* getBlockFromBucket(header* bucket, size_t size) {
    header* block = bucket;
    block = block->next;
    for (; block->size > 0; block = block->next) {
        if (block->size >= size) {
            removeFromBucket(block);
            return trimBlock(block, size);
        }
    }
    return NULL;
}

header* getBlockFromBuckets(size_t size) {
    for (unsigned int i = bucketIndexFromSize(size); i < NUM_BUCKETS; ++i) {
        header* block;
        if ((block = getBlockFromBucket(&BUCKETS[i], size))) {
            return block;
        }
    }
    return NULL;
}

void splitAfter(header* block, size_t size) {
    header* newBlock = (header*) ((unsigned long) block + size);
    
    updateSize(newBlock, block->size - size);

    newBlock->mapping = block->mapping;

    insertIntoBuckets(newBlock);
}

header* trimBlock(header* block, size_t size) {
    if (block->size - size < MIN_ALLOC) {
        return block;
    }

    splitAfter(block, size);
    updateSize(block, size);

    return block;
}

inline header* getBlockFromPtr(void* ptr) {
    return (header*) ((unsigned long) ptr - METADATA_OFFSET);
}

void insertIntoBuckets(header* inserted) {
    header* preInserted = &BUCKETS[bucketIndexFromSize(inserted->size)];

    if (inserted->size < 1024) {
        preInserted = preInserted->prev;
    } else {
        for (; true; preInserted = preInserted->next) {
            unsigned int nextBlockSize = preInserted->next->size;
            if (nextBlockSize == 0 || nextBlockSize > inserted->size) {
                break;
            }
        }
    }

    inserted->prev = preInserted;
    inserted->next = preInserted->next;
    inserted->next->prev = inserted;
    preInserted->next = inserted;
    inserted->free = true;
}

void* getMapping(size_t size) {
    void* mapping = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    
    if (mapping == MAP_FAILED) {
        errno = ENOMEM;
        return NULL;
    }

    for (unsigned short i = MAPPING_INDEX; i > 0; --i) {
        if ((unsigned long) mapping == MAPPINGS[i - 1][1]) {
            MAPPINGS[i - 1][1] += size;
            --MAPPING_INDEX;
            return mapping;
        }
    }

    if (MAPPING_INDEX == 1 << LOG2_NUM_MAPPINGS) {
        errno = ENOMEM;
        return NULL;
    }

    MAPPINGS[MAPPING_INDEX][0] = (unsigned long) mapping;
    MAPPINGS[MAPPING_INDEX][1] = (unsigned long) mapping + size;

    return mapping;
}

header* getBlockFromOS(size_t size) {
    size_t requestedSize = roundUpPower(size, MMAP_UNIT);

    if (requestedSize < size) {
        errno = EINVAL;
        return NULL;
    }

    header* mainBlock = getMapping(requestedSize);

    if (!mainBlock) {
        return NULL;
    }

    updateSize(mainBlock, requestedSize);

    mainBlock->mapping = MAPPING_INDEX++;

    return trimBlock(mainBlock, size);
}

void join(header* lower, header* higher) {
    removeFromBucket(lower);
    removeFromBucket(higher);
    updateSize(lower, lower->size + higher->size);
    insertIntoBuckets(lower);
}