#include <stdio.h>

static void helloWorld(void) {
    printf("Hello, World!\n");
}

static void byeWorld(void) {
    printf("Goodbye, World!\n");
}

void init(void) {
    helloWorld();
    byeWorld();
}
