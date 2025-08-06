#include <stdio.h>
#include <string.h>

/* Stub implementation for luaL_checkstring */
const char *luaL_checkstring(void *L, int arg) {
    printf("[lua] Would check string at arg %d\n", arg);
    return "gui";  /* Return a default callback name */
}

/* Stub implementation for luaL_openlibs */
void luaL_openlibs(void *L) {
    printf("[lua] Would open standard libraries\n");
}