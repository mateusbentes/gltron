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
