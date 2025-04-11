/* 
 * Implementation file for scripting functions
 */
#include <stdio.h>  /* For fprintf, stderr */
#include <stdlib.h> /* For malloc, free */
#include <string.h> /* For string operations */
#include "scripting/scripting.h" /* For function declarations */
#include "scripting/embedded_scripts.h" /* For get_embedded_script */

void scripting_RunString(const char *script) {
    if (!script) {
        fprintf(stderr, "[error] scripting_RunString: script is NULL\n");
        return;
    }
    
    /* Just print that we're executing the script */
    printf("[scripting] Script executed (stub)\n");
}

void runScript(int path, const char *name) {
    char *fullname;
    
    fullname = getPossiblePath(path, name);
    if(fullname != NULL) {
        if(nebu_FS_Test(fullname)) {
            printf("[scripting] Found script file: %s\n", fullname);
            
            /* Read the file content directly */
            FILE *f = fopen(fullname, "r");
            if (f) {
                /* Get file size */
                fseek(f, 0, SEEK_END);
                long size = ftell(f);
                fseek(f, 0, SEEK_SET);
                
                /* Allocate buffer for file content */
                char *buffer = (char *)malloc(size + 1);
                if (buffer) {
                    /* Read file content */
                    size_t read_size = fread(buffer, 1, size, f);
                    buffer[read_size] = '\0';  /* Null-terminate the string */
                    
                    /* Run the script */
                    printf("[scripting] Running script from file: %s\n", fullname);
                    scripting_RunString(buffer);
                    
                    /* Clean up */
                    free(buffer);
                } else {
                    fprintf(stderr, "[error] Failed to allocate memory for script: %s\n", fullname);
                }
                fclose(f);
            } else {
                fprintf(stderr, "[error] Cannot open script file: %s\n", fullname);
            }
            
            free(fullname);
            return;
        }
        free(fullname);
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

/* Shutdown the scripting system - simplified version */
void scripting_Shutdown(void) {
    printf("[scripting] Simplified scripting system shutdown\n");
}

/* Execute a simple command - simplified version */
int scripting_ExecuteCommand(const char *command) {
    if (!command) {
        fprintf(stderr, "[error] scripting_ExecuteCommand: command is NULL\n");
        return 0;
    }
    
    printf("[scripting] Would execute command: %s\n", command);
    return 1; /* Success */
}
