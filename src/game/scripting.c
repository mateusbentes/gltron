/* 
 * Implementation file for scripting functions
 */
#include <stdio.h>  /* For fprintf, stderr */
#include <stdlib.h> /* For malloc, free */
#include <string.h> /* For string operations */
#include "scripting/scripting.h" /* For function declarations */
#include "scripting/embedded_scripts.h" /* For get_embedded_script */

/* Global variable to store the Lua state */
static void *lua_state = NULL;

/* Get the Lua state */
void *scripting_GetLuaState(void) {
    return lua_state;
}

/* Set the Lua state */
void scripting_SetLuaState(void *L) {
    lua_state = L;
}

/* Stub implementation for luaL_dostring - renamed to avoid conflicts */
int stub_luaL_dostring(void *L, const char *str) {
    printf("[lua] Would execute: %s\n", str ? str : "(null)");
    return 0;  /* Success */
}

/* Stub implementation for lua_tostring - renamed to avoid conflicts */
const char *stub_lua_tostring(void *L, int index) {
    printf("[lua] Would get string at index %d\n", index);
    return "error message stub";
}

/* Stub implementation for lua_pop - renamed to avoid conflicts */
void stub_lua_pop(void *L, int n) {
    printf("[lua] Would pop %d elements from stack\n", n);
}

/* Stub implementation for lua_close - renamed to avoid conflicts */
void stub_lua_close(void *L) {
    printf("[lua] Would close Lua state\n");
}

void scripting_RunString(const char *script) {
    void *L = scripting_GetLuaState();
    if (!L) {
        fprintf(stderr, "[error] scripting_RunString: Lua state is NULL\n");
        return;
    }
    
    if (!script) {
        fprintf(stderr, "[error] scripting_RunString: script is NULL\n");
        return;
    }
    
    /* Execute the script using our stub implementation */
    printf("[scripting] Would run script: %s\n", script ? script : "(null)");
    
    /* We don't actually execute the script, just pretend we did */
    /* if (stub_luaL_dostring(L, script) != 0) {
        fprintf(stderr, "[error] scripting_RunString: %s\n", stub_lua_tostring(L, -1));
        stub_lua_pop(L, 1);
    } */
}

void runScript(int path, const char *name) {
    char *fullname = NULL;
    FILE *f = NULL;
    char *buffer = NULL;
    long size = 0;
    size_t read_size = 0;
    
    printf("[script] Running script: %s from path %d\n", name ? name : "(null)", path);
    
    if (!name) {
        fprintf(stderr, "[error] runScript: name is NULL\n");
        return;
    }
    
    fullname = getPossiblePath(path, name);
    if(fullname == NULL) {
        fprintf(stderr, "[error] runScript: couldn't get path for %s\n", name);
        return;
    }
    
    /* Check if fullname is valid before printing */
    if (fullname) {
        printf("[script] Full path: %s\n", fullname);
    } else {
        printf("[script] Full path: (null)\n");
    }
    
    if(fullname && nebu_FS_Test(fullname)) {
        printf("[script] File exists, running script\n");
        
#ifdef USE_EMBEDDED_SCRIPTS
        /* Use embedded script instead of file */
        const char* script = get_embedded_script(name);
        if(script) {
            printf("[script] Using embedded script: %s\n", name);
            scripting_RunString(script);
        } else {
            fprintf(stderr, "[error] Embedded script not found: %s\n", name);
        }
#else
        /* Use file */
        f = fopen(fullname, "r");
        if (!f) {
            fprintf(stderr, "[error] Cannot open script file: %s\n", fullname);
            free(fullname);
            return;
        }
        
        /* Get file size */
        if (fseek(f, 0, SEEK_END) != 0) {
            fprintf(stderr, "[error] Failed to seek to end of file: %s\n", fullname);
            fclose(f);
            free(fullname);
            return;
        }
        
        size = ftell(f);
        if (size < 0) {
            fprintf(stderr, "[error] Failed to get file size: %s\n", fullname);
            fclose(f);
            free(fullname);
            return;
        }
        
        if (fseek(f, 0, SEEK_SET) != 0) {
            fprintf(stderr, "[error] Failed to seek to start of file: %s\n", fullname);
            fclose(f);
            free(fullname);
            return;
        }
        
        /* Allocate buffer for file content */
        buffer = (char *)malloc(size + 1);
        if (!buffer) {
            fprintf(stderr, "[error] Failed to allocate memory for script: %s\n", fullname);
            fclose(f);
            free(fullname);
            return;
        }
        
        /* Read file content */
        read_size = fread(buffer, 1, size, f);
        if (read_size < (size_t)size) {
            fprintf(stderr, "[warning] Read fewer bytes than expected: %zu < %ld\n", read_size, size);
        }
        
        buffer[read_size] = '\0';  /* Null-terminate the string */
        
        /* Run the script */
        printf("[scripting] Running script from file: %s\n", fullname);
        scripting_RunString(buffer);
        
        /* Clean up */
        free(buffer);
        fclose(f);
#endif
        
        free(fullname);
    } else {
        if (fullname) {
            fprintf(stderr, "[error] runScript: %s not found at %s\n", name, fullname);
            free(fullname);
        } else {
            fprintf(stderr, "[error] runScript: %s not found (fullname is NULL)\n", name);
        }
        
        /* If we get here, the script file wasn't found, try embedded scripts */
        printf("[scripting] Script file not found, trying embedded script: %s\n", name);
        const char *script_content = get_embedded_script(name);
        if(script_content != NULL) {
            printf("[scripting] Running embedded script: %s\n", name);
            scripting_RunString(script_content);
        } else {
            fprintf(stderr, "[error] Script not found (neither file nor embedded): %s\n", name);
        }
    }
}

/* Shutdown the scripting system - simplified version */
void scripting_Shutdown(void) {
    void *L = scripting_GetLuaState();
    if (L) {
        /* Use our stub implementation */
        stub_lua_close(L);
        lua_state = NULL;
    }
    printf("[scripting] Scripting system shutdown\n");
}

/* Execute a simple command - simplified version */
int scripting_ExecuteCommand(const char *command) {
    void *L = scripting_GetLuaState();
    if (!L) {
        fprintf(stderr, "[error] scripting_ExecuteCommand: Lua state is NULL\n");
        return 0;
    }
    
    if (!command) {
        fprintf(stderr, "[error] scripting_ExecuteCommand: command is NULL\n");
        return 0;
    }
    
    /* Execute the command using our stub implementation */
    printf("[scripting] Would execute command: %s\n", command ? command : "(null)");
    return 1; /* Success */
}
