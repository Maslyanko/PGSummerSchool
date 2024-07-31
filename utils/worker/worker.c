#define _GNU_SOURCE
#include <sched.h>
#include <linux/sched.h>
#include <stdio.h>
#include <unistd.h>
#include <dlfcn.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>


#include "worker.h"
#include "../alloc/alloc.h"
#include "../workso/workso.h"
#include "../svc/svc.h"
#include "../logger/logger.h"

typedef struct WorkerMetaData {
    Worker* worker;
    void* stackPtr;
    pid_t pid;
} WorkerMetaData;

List* WORKERS = NULL;

void registerWorker(Worker* worker) {
    if (!WORKERS) {
        WORKERS = (List*) myMalloc(sizeof(List));
        initList(WORKERS);
    }

    WorkerMetaData* workerData = (WorkerMetaData*)myMalloc(sizeof(WorkerMetaData)); 

    workerData->worker = (Worker*) myMalloc(sizeof(Worker));

    workerData->worker->libName = (char*) myMalloc(strlen(worker->libName) + 1);
    workerData->worker->mainName = (char*) myMalloc(strlen(worker->mainName) + 1);
    workerData->worker->description = (char*) myMalloc(strlen(worker->description) + 1);
    workerData->worker->name = (char*) myMalloc(strlen(worker->name) + 1);
    workerData->worker->argv = worker->argv;
    workerData->worker->argc = worker->argc;

    strcpy(workerData->worker->libName, worker->libName);
    strcpy(workerData->worker->mainName, worker->mainName);
    strcpy(workerData->worker->description, worker->description);
    strcpy(workerData->worker->name, worker->name);
    
    workerData->stackPtr = NULL;
    workerData->pid = 0;

    pushBack(WORKERS, workerData);
}

void launchWorkers() {
    if (!WORKERS) {
        return;
    }

    ListNode* current = WORKERS->head;

    errno = 0;

    while (current) {
        void* libraryPtr = getLibraryPtr(((WorkerMetaData*)current->data)->worker->libName);

        if (libraryPtr) {
            int (*workerMain) (void*) = dlsym(libraryPtr, ((WorkerMetaData*)current->data)->worker->mainName);

            if (workerMain) {
                Variable* stackSize = getVariable("NEW_PROCESS_STACK_SIZE_KB");
    
                *(int*)stackSize->value *= 1024;
                void* stack = myMalloc(*(int*)stackSize->value);

                ((WorkerMetaData*)current->data)->pid
                 = 
                clone
                (
                    workerMain,
                    stack + *(int*)stackSize->value, 
                    SIGCHLD | CLONE_FS, 
                    (void*)((WorkerMetaData*)current->data)->worker->argv
                );

                if (((WorkerMetaData*)current->data)->pid == -1) {
                    elog(ERROR, strerror(errno));
                    myFree(stack);
                    errno = 0;
                    current = current->next;
                    continue;
                }

                ((WorkerMetaData*)current->data)->stackPtr = stack;
            }
        }

        current = current->next;
    }
}

void finishWorkers() {
    if (!WORKERS) {
        return;
    }

    ListNode* current = WORKERS->head;

    while (current) {
        int status;

        waitpid(((WorkerMetaData*)current->data)->pid, &status, 0);

        if (((WorkerMetaData*)current->data)->stackPtr) {
            myFree(((WorkerMetaData*)current->data)->stackPtr);
        }

        myFree(((WorkerMetaData*)current->data)->worker->libName);
        myFree(((WorkerMetaData*)current->data)->worker->mainName);
        myFree(((WorkerMetaData*)current->data)->worker->description);
        myFree(((WorkerMetaData*)current->data)->worker->name);
        myFree(((WorkerMetaData*)current->data)->worker);
        myFree(current->data);
        current = current->next;
    }

    freeList(WORKERS);
}