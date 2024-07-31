#include <stdio.h>

#include "../../utils/worker/worker.h"
#include "../../utils/alloc/alloc.h"
#include "../../utils/shmem/shmem.h"

Worker* worker;

void init(void) {
    worker = (Worker*) myMalloc(sizeof(Worker));
    worker->libName = "secondWorker.so";
    worker->description = "";
    worker->mainName = "workerMain";
    worker->name = "Nike";
    worker->argc = 0;
    worker->argv = NULL;
    registerWorker(worker);

    requestShmem("example", sizeof(int));
}

void workerMain() {
    sleep(2);
    int* sharedInt = (int*) getShmemPtr("example");

    printf("Second worker: %d\n", *sharedInt);
}