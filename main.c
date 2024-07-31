#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <signal.h>

#include "utils/workso/workso.h"
#include "utils/logger/logger.h"
#include "utils/svc/svc.h"
#include "utils/config/config.h"
#include "utils/cache/cache.h"
#include "utils/alloc/alloc.h"
#include "utils/worker/worker.h"
#include "utils/proxy/proxy.h"
#include "utils/shmem/shmem.h"

int TIME_TO_FINISH_MAIN = 0;

void mainProcHandler(int signal) {
    TIME_TO_FINISH_MAIN = 1;
}

int main(int argc, char **argv) {
    struct sigaction mainFinishAction;
    mainFinishAction.sa_handler = mainProcHandler;
    sigaction(SIGINT, &mainFinishAction, NULL);

    parseConfig();

    Variable* loggerPath = getVariable("PATH_TO_LOGGER_FILE");
    Variable* logFileCapacity = getVariable("LOGGER_CAPACITY");
    Variable* pathToPluginsDir = getVariable("PATH_TO_PLUGINS_DIRECTORY");

    initLogger((char*)loggerPath->value, LOG_CLEAR, *(int*)logFileCapacity->value);

    loadSharedLibraries((char*)pathToPluginsDir->value);
    launchLibraries();
    launchShmem();
    launchWorkers();
    launchProxy();

    myFree(pathToPluginsDir->name);
    myFree(pathToPluginsDir->value);
    myFree(pathToPluginsDir);
    myFree(loggerPath->value);
    myFree(loggerPath->name);
    myFree(loggerPath);
    myFree(logFileCapacity->name);
    myFree(logFileCapacity->value);
    myFree(logFileCapacity);

    while (1) {
        if (TIME_TO_FINISH_MAIN) {
            break;
        }
        //Здесь неплохо бы добавть какие-нибудь проверки на жизнь всяких штук.
    }
    
    finishProxy();
    finishWorkers();
    finishCVS();
    closeSharedLibraries();
    finishLogger();

    return 0;
}
