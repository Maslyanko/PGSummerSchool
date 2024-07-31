#include <stdio.h>
#include <dlfcn.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "workso.h"
#include "../utils/utils.h"
#include "../logger/logger.h"
#include "../alloc/alloc.h"
#include "../worker/worker.h"
#include "../utils/hashtable.h"

HashTable* LIB_TABLE;

int loadSharedLibraries(const char* sharedDir) {
    DIR* directory = opendir(sharedDir);

    if (!directory) {
        elog(ERROR, "Couldn't open a directory with shared libraries;");
        return -1;
    }

    elog(INFO, "Loading shared libraries;");

    LIB_TABLE = createHashTable(DEFAULT_HASHTABLE_SIZE, RESIZE_HASHTABLE_SCALE, HASHTABLE_BUCKET_CAPACITY);

    struct dirent* entry;

    while ((entry = readdir(directory))) {
        if (!strcmp(".", entry->d_name) || !strcmp("..", entry->d_name)) {
            continue;
        }

        char* fullPath = makeFullName(2, DIRECTORY_TYPE, sharedDir, entry->d_name);
        DIR* subDirectory = opendir(fullPath);
        myFree(fullPath);

        if (!subDirectory) {
            continue;
        }

        struct dirent* subEntry;
        while ((subEntry = readdir(subDirectory)) != NULL) {
            if (!strcmp(".", subEntry->d_name) || !strcmp("..", subEntry->d_name)) {
                continue;
            }
        
            int length = strlen(subEntry->d_name);
        
            if (
                subEntry->d_name[length - 3] == '.' &&
                subEntry->d_name[length - 2] == 's' &&
                subEntry->d_name[length - 1] == 'o'
            ) {
                char* fileName = makeFullName(3, FILE_TYPE, sharedDir, entry->d_name, subEntry->d_name);
                void* ptr = dlopen(fileName, RTLD_NOW | RTLD_GLOBAL);

                if (ptr == NULL) {
                    printf("%s\n", dlerror());
                    sprintf(fileName, "%s could not be loaded;", subEntry->d_name);
                    elog(ERROR, fileName);
                } else {
                    char* name = (char*) myMalloc(strlen(subEntry->d_name) + 1);
                    strcpy(name, subEntry->d_name);

                    insert(LIB_TABLE, name, ptr);
                    sprintf(fileName, "%s loaded successfully;", subEntry->d_name);
                    elog(INFO, fileName);
                }

                myFree(fileName);
            }
        }

        closedir(subDirectory);
    }

    closedir(directory);
    elog(INFO, "End of libraries loading");
    return 0;
}

void closeSharedLibraries() {
    elog(INFO, "Closing shared libraries;");

    HashTableEntry* libraries = getAllElements(LIB_TABLE);
    
    for (int i = 0; i < LIB_TABLE->count; ++i) {
        void* ptr = libraries[i].data;
        char* name = libraries[i].key;
        dlclose(ptr);

        char* message = (char*) myMalloc(strlen(name) + 40);
        sprintf(message, "%s unloaded successfully;", name);
        elog(INFO, message);

        myFree(message);
    }

    myFree(libraries);
    freeTable(LIB_TABLE);

    elog(INFO, "End of closing shared libraries");
}

void launchLibraries() {
    elog(INFO, "Launching shared libraries;");

    HashTableEntry* libraries = getAllElements(LIB_TABLE);

    for (int i = 0; i < LIB_TABLE->count; ++i) {
        void (*init)(void) = dlsym(libraries[i].data, "init");
        
        if (init != NULL) {
            init();
        } else {
            char* error = (char*) myMalloc(strlen(libraries[i].key) + 32);
            sprintf(error, "Couldn't find launch function in %s;", libraries[i].key);
            elog(ERROR, error);
            myFree(error);
        }
    }

    myFree(libraries);

    elog(INFO, "End of libraries launching;");
}

void* getLibraryPtr(char* name) {
    return getDataElement(LIB_TABLE, name);
}