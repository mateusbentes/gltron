#include "filesystem/path.h"
#include "Nebu_scripting.h"

#include <stdio.h>

#include "base/nebu_debug_memory.h"

// Function to get embedded script content
extern const char* get_embedded_script(const char *name);

int runScript(int ePath, const char *name) {
    lua_State *L = scripting_GetLuaState();
    if (!L) {
        fprintf(stderr, "[error] Lua state is NULL\n");
        return -1;
    }

    // Check for embedded script
    const char *embeddedScript = get_embedded_script(name);
    if (embeddedScript) {
        fprintf(stderr, "[script] Running embedded script '%s'\n", name);
        if (luaL_loadbuffer(L, embeddedScript, strlen(embeddedScript), name) != 0 ||
            lua_pcall(L, 0, 0, 0) != 0) {
            fprintf(stderr, "[error] Failed to execute embedded script '%s': %s\n", name, lua_tostring(L, -1));
            lua_pop(L, 1);
            return -1;
        }
        fprintf(stderr, "[script] Finished running embedded script '%s'\n", name);
        return 0;
    }

    // Fallback to filesystem
    char *s = getPath(ePath, name);
    if (!s) {
        fprintf(stderr, "[error] Failed to construct path for script '%s'\n", name);
        return -1;  // Indicate failure
    }

    fprintf(stderr, "[script] Start running script '%s'\n", name);
    int result = scripting_RunFile(s);
    if (result != 0) {
        fprintf(stderr, "[error] Failed to run script '%s'\n", name);
    } else {
        fprintf(stderr, "[script] Finished running script '%s'\n", name);
    }

    free(s);
    return result;  // Return the result of scripting_RunFile
}
