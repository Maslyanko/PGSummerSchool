#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>

#include "config.h"
#include "../svc/svc.h"
#include "../alloc/alloc.h"

#define CONFIG_FILE "proxy.conf"

size_t CONFIG_SIZE;
char* START_FILE;

int isDgigit(char symbol) {
    return symbol >= '0' && symbol <= '9';
}

int isLetter(char symbol) {
    return (symbol >= 'a' && symbol <= 'z') || (symbol >= 'A' && symbol <= 'Z');
}

void parseConfig(void) {
    int fd = open("proxy.conf", O_RDONLY);

    if (fd == -1) {
        printf("Error opening config file: %s\n", strerror(errno));
        return;
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        close(fd);
        printf("Error getting file size: %s\n", strerror(errno));
        return;
    }

    CONFIG_SIZE = st.st_size;

    char* file = (char*) mmap(NULL, CONFIG_SIZE, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    if (!file) {
        printf("Error mapping file to memory: %s\n", strerror(errno));
        return;
    }

    for (size_t i = 0; i < CONFIG_SIZE; ++i) {
        if (file[i] == '#') {
            for (; i < CONFIG_SIZE && file[i] != '\n'; ++i);
            if (file[i] == '\n') {
                --i;
            }
        }

        if (i == 0 || file[i] == '\n' || isLetter(file[i])) {
            if (file[i] == '\n') {
                ++i;
            }
            char* startOfName = &(file[i]);

            for (; i < CONFIG_SIZE && file[i] != '=' && file[i] != ' '; ++i);

            char* name = (char*) myMalloc(&(file[i]) - startOfName + 1);
            memcpy(name, startOfName, &(file[i]) - startOfName);
            name[&(file[i]) - startOfName] = '\0';
            
            for (; i < CONFIG_SIZE && (file[i] == ' ' || file[i] == '='); ++i);
            
            if (file[i] == '\'') {
                ++i;
                char* startOfValue = &(file[i]);

                for (; i < CONFIG_SIZE && file[i] != '\''; ++i);

                char* value = (char*) myMalloc(&(file[i]) - startOfValue + 1);
                memcpy(value, startOfValue, &(file[i]) - startOfValue);
                value[&(file[i]) - startOfValue] = '\0';
                
                registerVariable(name, STRING, value);

                myFree(name);
                myFree(value);
            } 
            else {
                char* startOfValue = &(file[i]);

                while (
                    i < CONFIG_SIZE && 
                    file[i] != ' ' && 
                    file[i] != '\n' && 
                    file[i] != '#' && 
                    isDgigit(file[i])
                ) {
                    ++i;
                }

                char* ptr = startOfValue + 1;
                int numericValue = strtol(startOfValue, &ptr, 0);
            
                registerVariable(name, INT, &numericValue);

                myFree(name);
            }
        }
    }

    munmap(file, (size_t)st.st_size);
}