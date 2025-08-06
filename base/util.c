#include "config.h"

#ifdef ANDROID
#include "android_config.h"
#else
#include "platform_config.h"
#endif

#include "filesystem/path.h"
#include "Nebu_scripting.h"

#include <stdio.h>

#include "base/nebu_debug_memory.h"

void runScript(int ePath, const char *name) {
    char *s;
    s = getPath(ePath, name);
	fprintf(stderr, "[script] start running script \'%s\'\n", name);
    scripting_RunFile(s);
	fprintf(stderr, "[script] finished running script \'%s\'\n", name);
    free(s);
}



// Stub for scripting_RunFile
int scripting_RunFile(const char* filename) {
    // LOGI("scripting_RunFile stub called for: %s", filename); // Removed to avoid redefinition
    return 0; // Indicate failure or success as needed
}
