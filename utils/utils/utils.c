#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"
#include "../alloc/alloc.h"

char* makeFullName(int number, int flags, ...) {
    int totalLength = 0;
    va_list args;

    va_start(args, flags);
        for (int i = 0; i < number; ++i) {
            totalLength += strlen(va_arg(args, const char*)) + 1;
        }
    va_end(args);

    char* fullPath = (char*) myCalloc(totalLength, 1);

    va_start(args, flags);
        for (int i = 0; i < number; ++i) {
            const char* name = va_arg(args, const char*);
            strcat(fullPath, name);

            if (name[strlen(name) - 1] != '/') {
                strcat(fullPath, "/");
            }
        }
    va_end(args);

    if (flags == FILE_TYPE) {
        fullPath[strlen(fullPath) - 1] = '\0';
    }

    return fullPath;
}