#include <stdio.h>

#include "utils/workso/workso.h"

int main(int argc, char **argv) {
    loadSharedLibraries("plugins/");
    
    launchLibraries();

    closeSharedLibraries();
    return 0;
}
