#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <sys/mman.h>

#include "logger.h"
#include "../utils/utils.h"
#include "../alloc/alloc.h"

sem_t* SEMAPHORE;
int NEW_LOGGER_FD = 0;
int OLD_LOGGER_FD = 0;
int LOGGER_CAPACITY = 0;
int DEFAULT_CAPACITY = 128;
int CURRENT_SIZE = 0;
int IS_CYCLIC = 0;
int PID_CREATOR;
char* FILE_PATH;
char* FRESH_LOGS_NAME = NULL;

int removeLines(const char* fileName, int oldFd, int capacity) {
    bool nextLineIsFirst = false;
    long firstLinePosition = 0; 
    int length, count = 0, bufferSize = 1024;
    char* buffer = (char*) myMalloc(bufferSize * sizeof(char));


    lseek(oldFd, 0, SEEK_END);

    while (true) {
        long position = lseek(oldFd, -bufferSize, SEEK_CUR);

        if (position == -1) {
            length = lseek(oldFd, 0, SEEK_CUR);
            lseek(oldFd, 0, SEEK_SET);
            length = read(oldFd, buffer, length);
            position = 0;
        } else {
            length = read(oldFd, buffer, bufferSize);
        }
        
        for (int i = length - 1; i >= 0; --i) {
            if (buffer[i] == '\n') {
                ++count;
                if (nextLineIsFirst) {
                    firstLinePosition = position + i + 1;
                    break;
                }
            }
            if (count == capacity) {
                nextLineIsFirst = true;
            }
            if (count > capacity) {
                break;
            }
        }

        if (count > capacity) {
            break;
        }

        if (lseek(oldFd, -bufferSize, SEEK_CUR) == -1) {
            break;
        }
    }

    if (count < capacity) {
        CURRENT_SIZE = count;
        return oldFd;
    }
    CURRENT_SIZE = capacity;


    char* newName = (char*) myMalloc(strlen(fileName) + 4);
    strcpy(newName, fileName);
    strcpy(newName + strlen(fileName), "_helper");

    struct stat fileStat;
    fstat(oldFd, &fileStat);

    int newFd = open(newName, O_RDWR | O_CREAT, fileStat.st_mode);

    lseek(oldFd, firstLinePosition, SEEK_SET);

    while ((length = read(oldFd, buffer, bufferSize)) > 0) {
        write(newFd, buffer, length);
    }


    close(oldFd);
    remove(fileName);
    rename(newName, fileName);
    myFree(newName);
    myFree(buffer);

    return newFd;
}

void elog(LOG_LEVEL level, const char* message) {
    sem_wait(SEMAPHORE);

    if (IS_CYCLIC) {
        if (CURRENT_SIZE == LOGGER_CAPACITY) {
            if (!FRESH_LOGS_NAME) {
                FRESH_LOGS_NAME = (char*) myMalloc(strlen(FILE_PATH) + 7);
                strcpy(FRESH_LOGS_NAME, FILE_PATH);
                strcpy(FRESH_LOGS_NAME + strlen(FILE_PATH), "_fresh");   
            }

            OLD_LOGGER_FD = NEW_LOGGER_FD;

            struct stat fileStat;
            fstat(NEW_LOGGER_FD, &fileStat);
        
            NEW_LOGGER_FD = open(FRESH_LOGS_NAME, O_RDWR | O_CREAT, fileStat.st_mode);                       
        } else if (CURRENT_SIZE == 2 * LOGGER_CAPACITY - 1) {
            close(OLD_LOGGER_FD);
            remove(FILE_PATH);
            rename(FRESH_LOGS_NAME, FILE_PATH);

            CURRENT_SIZE = LOGGER_CAPACITY - 1;
        }
    }

    char str[19];
    sprintf(str, "[%d]: ", getpid());
    write(NEW_LOGGER_FD, str, strlen(str));

    time_t now = time(NULL);
    struct tm* data = localtime(&now);

    strftime(str, sizeof(str), "%D, %T", data);
    write(NEW_LOGGER_FD, str, 18);

    write(NEW_LOGGER_FD, " [", 2);
    switch (level) {
        case FATAL:
            write(NEW_LOGGER_FD, "FATAL] ", 7);
            break;
        case ERROR:
            write(NEW_LOGGER_FD, "ERROR] ", 7);
            break;
        case WARNING:
            write(NEW_LOGGER_FD, "WARNING] ", 9);
            break;
        case INFO:
            write(NEW_LOGGER_FD, "INFO] ", 6);
            break;
        case DEBUG:
            write(NEW_LOGGER_FD, "DEBUG] ", 7);
            break;
        default:
            break;
    }
    
    write(NEW_LOGGER_FD, message, strlen(message));
    write(NEW_LOGGER_FD, "\n", 1);

    if (IS_CYCLIC) {
        ++CURRENT_SIZE;
    }

    sem_post(SEMAPHORE);
}

int initLogger(const char* filePath, int flags, int capacity) {
    if (NEW_LOGGER_FD) {
        return -1;
    }

    bool isClear = false;
    errno = 0;

    if (flags >= 4) {
        IS_CYCLIC = 1;
        flags -= 4;
    }

    if (capacity > 0) {
        LOGGER_CAPACITY = capacity;
    } 
    else if (IS_CYCLIC) {
        LOGGER_CAPACITY = DEFAULT_CAPACITY;
    }
    
    if ((flags == 0 || flags == 1) && !IS_CYCLIC) {
        NEW_LOGGER_FD = open(filePath, O_APPEND | O_RDWR | O_CREAT, S_IWUSR | S_IRUSR | S_IROTH | S_IRGRP);
    } 
    else if (flags == 2) {
        NEW_LOGGER_FD = open(filePath, O_TRUNC | O_RDWR | O_CREAT, S_IWUSR | S_IRUSR | S_IROTH | S_IRGRP);
        isClear = true;
    }
    else if (IS_CYCLIC) {
        NEW_LOGGER_FD = open(filePath, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR | S_IROTH | S_IRGRP);
    }    
    else {
        printf("The logger does not support this set of flags;\n");
        return -1;
    }

    if (errno) {
        printf("%s;\n", strerror(errno));
        return -1;
    }


    if ((IS_CYCLIC && !isClear) || LOGGER_CAPACITY) {
        NEW_LOGGER_FD = removeLines(filePath, NEW_LOGGER_FD, capacity);
    }

    FILE_PATH = (char*) myMalloc(strlen(filePath) + 1);
    strcpy(FILE_PATH, filePath);
    PID_CREATOR = getpid();

    SEMAPHORE = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0);

    sem_init(SEMAPHORE, 1, 1);

    return 0;
}

int finishLogger() {
    if (getpid() != PID_CREATOR) {
        return -1;
    }
    
    if (IS_CYCLIC) {
        char* finalFileName = (char*) myMalloc(strlen(FILE_PATH) + 7);
        strcpy(finalFileName, FILE_PATH);
        strcpy(finalFileName + strlen(FILE_PATH), "_final");

        struct stat fileStat;
        fstat(NEW_LOGGER_FD, &fileStat);

        int finalFd = open(finalFileName, O_RDWR | O_CREAT, fileStat.st_mode);

        int length, position = -1, bufferSize = 1024, newCount = 0;
        char* buffer = (char*) myMalloc(bufferSize);

        lseek(NEW_LOGGER_FD, 0, SEEK_SET);

        while ((length = read(NEW_LOGGER_FD, buffer, bufferSize)) > 0) {
            for (int i = 0; i < length; ++i) {
                if (buffer[i] == '\n') {
                    ++newCount;
                }
            }
            write(finalFd, buffer, length);
        }

        if (OLD_LOGGER_FD) {
            int oldCount = 0;

            lseek(OLD_LOGGER_FD, 0, SEEK_SET);

            while ((length = read(OLD_LOGGER_FD, buffer, bufferSize)) > 0) {
                for (int i = 0; i < length; ++i) {
                    if (buffer[i] == '\n') {
                        ++oldCount;
                    }
                    if (newCount == oldCount) {
                        position = lseek(OLD_LOGGER_FD, 0, SEEK_CUR) - length + i + 1;
                        break;
                    }
                }
                if (position != -1) {
                    break;
                }
            }

            lseek(OLD_LOGGER_FD, position, SEEK_SET);

            while ((length = read(OLD_LOGGER_FD, buffer, bufferSize)) > 0) {
                write(finalFd, buffer, length);
            }

            close(OLD_LOGGER_FD);
        }
        remove(FILE_PATH);
        remove(FRESH_LOGS_NAME);
        rename(finalFileName, FILE_PATH);

        close(finalFd);
        myFree(finalFileName);
    }

    errno = 0;

    if (NEW_LOGGER_FD) {
        close(NEW_LOGGER_FD);
    }

    if (errno) {
        printf("%s\n;", strerror(errno));
        return -1;
    }

    myFree(FILE_PATH);

    sem_destroy(SEMAPHORE);

    return 0;
}

