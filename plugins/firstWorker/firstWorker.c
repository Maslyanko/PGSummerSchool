#include <stdio.h>

#include "../../utils/alloc/alloc.h"
#include "../../utils/shmem/shmem.h"
#include "../../utils/worker/worker.h"

Worker* worker;

void init(void) {
    worker = (Worker*) myMalloc(sizeof(Worker));
    worker->libName = "firstWorker.so";
    worker->description = "";
    worker->mainName = "workerMain";
    worker->name = "Bob";
    worker->argc = 0;
    worker->argv = NULL;
    registerWorker(worker);

    requestShmem("example", sizeof(int));
}

void workerMain() {
    int* sharedInt = (int*) getShmemPtr("example");
    
    *sharedInt = 10;
    
    printf("First worker: %d\n", *sharedInt);
}