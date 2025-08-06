#ifndef NEBU_DEBUG_MEMORY_H
#define NEBU_DEBUG_MEMORY_H

// Android stub for nebu_debug_memory.h
#include <stdlib.h>

// Memory debugging stubs - just use standard malloc/free for Android
#define malloc_track(size) malloc(size)
#define free_track(ptr) free(ptr)
#define realloc_track(ptr, size) realloc(ptr, size)

void memory_debug_init(void);
void memory_debug_shutdown(void);
void memory_debug_report(void);
void nebu_debug_memory_CheckLeaksOnExit(void);

#endif // NEBU_DEBUG_MEMORY_H

