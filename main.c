#include <stdio.h>
#include <unistd.h>

#include "utils/workso/workso.h"
#include "utils/logger/logger.h"
#include "utils/svc/svc.h"
#include "utils/config/config.h"
#include "utils/cache/cache.h"
#include "utils/alloc/alloc.h"

int main(int argc, char **argv) {
    parseConfig();

    Variable* loggerPath = getVariable("PATH_TO_LOGGER_FILE");
    Variable* logFileCapacity = getVariable("LOGGER_CAPACITY");
    Variable* pathToPluginsDir = getVariable("PATH_TO_PLUGINS_DIRECTORY");

    initLogger((char*)loggerPath->value, LOG_CLEAR | LOG_CYCLIC_WRITE, *(int*)logFileCapacity->value);

    loadSharedLibraries((char*)pathToPluginsDir->value);
    launchLibraries();

    finishCache();
    finishCVS();
    closeSharedLibraries();
    finishLogger();

    myFree(pathToPluginsDir->name);
    myFree(pathToPluginsDir->value);
    myFree(pathToPluginsDir);

    myFree(loggerPath->value);
    myFree(loggerPath->name);
    myFree(loggerPath);
    myFree(logFileCapacity->name);
    myFree(logFileCapacity->value);
    myFree(logFileCapacity);

    return 0;
}
