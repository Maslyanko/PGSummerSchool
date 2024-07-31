#define _GNU_SOURCE
#include <sched.h>
#include <linux/sched.h>
#include <stdio.h>
#include <unistd.h>
#include <dlfcn.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>

#include "proxy.h"
#include "../logger/logger.h"
#include "../cache/cache.h"
#include "../svc/svc.h"
#include "../utils/list.h"
#include "../alloc/alloc.h"

typedef struct ClientWorkerData {
    pid_t pid;
    int clientFd;
    void* stack;
} ClientWorkerData;

List* CLIENT_WORKERS = NULL;
pid_t PROXY_PID = 0;

void proxyFinishHandler(int signal);

static int clientWorkerMain(void* arg) {
    char buffer[1024];
    int clientFd = *(int*) arg;

    while (1) {
        ssize_t length;
        if ((length = read(clientFd, buffer, 1024)) == -1) {
            elog(ERROR, "Failed to read from client socket");
            break;
        }

        buffer[length] = '\0';

        char* response = getDataFromCache(buffer);

        if (response == NULL) {
            int result = atoi(buffer);
            result *= result;
            
            response = (char*) myMalloc(20);
            sprintf(response, "%d", result);

            char* request = (char*) myMalloc(strlen(buffer) + 1);
            strcpy(request, buffer);

            insertDataIntoCache(request, response);
            sleep(3);
        }
        write(clientFd, response, strlen(response));
    }

    return 0;
}

void launchProxy() {
    if (CLIENT_WORKERS) {
        return;
    }

    pid_t PROXY_PID = fork();
    if (PROXY_PID < 0) {
        elog(ERROR, "Failed to start the proxy process;");
        return;
    } else if (PROXY_PID > 0) {
        elog(INFO, "Starting proxy process successfully;");
        return;
    }

    struct sigaction proxyFinishAction;
    proxyFinishAction.sa_handler = proxyFinishHandler;
    sigaction(SIGINT, &proxyFinishAction, NULL);

    Variable* port = getVariable("PROXY_PORT");
    Variable* ip = getVariable("PROXY_IP");
    Variable* clientsNumber = getVariable("PROXY_CLIENT_NUMBER");
    Variable* cacheSize = getVariable("CACHE_SIZE");

    initCache(*(int*)cacheSize->value);

    int proxyFd, clientFd;
    struct sockaddr_in proxyAddress, clientAddress;
    socklen_t addrLen = sizeof(clientAddress);

    if ((proxyFd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        elog(ERROR, strerror(errno));
        return;
    }

    proxyAddress.sin_family = AF_INET;
    proxyAddress.sin_port = htons(*(int*)port->value);
    proxyAddress.sin_addr.s_addr = inet_addr((char*)ip->value);

    if (bind(proxyFd, (struct sockaddr*)&proxyAddress, sizeof(proxyAddress)) == -1) {
        elog(ERROR, strerror(errno));
        return;
    }

    if (listen(proxyFd, *(int*)clientsNumber->value) == -1) {
        elog(ERROR, strerror(errno));
        return;
    }

    CLIENT_WORKERS = (List*) myMalloc(sizeof(List));
    initList(CLIENT_WORKERS);

    myFree(port->name);
    myFree(port->value);
    myFree(port);
    myFree(ip->name);
    myFree(ip->value);
    myFree(ip);
    myFree(clientsNumber->name);
    myFree(clientsNumber->value);
    myFree(clientsNumber);
    myFree(cacheSize->name);
    myFree(cacheSize->value);
    myFree(cacheSize);

    while (1) {
        if ((clientFd = accept(proxyFd, (struct sockaddr*) &clientAddress, &addrLen)) == -1) {
            elog(ERROR, strerror(errno));
            continue;
        }

        void* stack = myMalloc(8192);

        pid_t pid = clone(&clientWorkerMain, stack + 8192, CLONE_VM | CLONE_FS, &clientFd);

        if (pid < 0) {
            elog(ERROR, "Failed to create proxy client worker process;");
            myFree(stack);
            close(clientFd);
            continue;
        }

        ClientWorkerData* data = myMalloc(sizeof(ClientWorkerData));
        data->pid = pid;
        data->clientFd = clientFd;
        data->stack = stack;

        pushBack(CLIENT_WORKERS, data);
    }
}


void proxyFinishHandler(int signal) {
    ListNode* current = CLIENT_WORKERS->head;

    while (current) {
        ClientWorkerData* worker = (ClientWorkerData*) current->data;

        kill(worker->pid, SIGINT);
        waitpid(worker->pid, NULL, 0);
        myFree(worker->stack);

        current = current->next;
    }

    freeList(CLIENT_WORKERS);

    finishCache();

    elog(INFO, "Proxy finished successfully;");
    exit(EXIT_SUCCESS);
}

void finishProxy() {
    if (CLIENT_WORKERS) {
        kill(PROXY_PID, SIGINT);
    }
}