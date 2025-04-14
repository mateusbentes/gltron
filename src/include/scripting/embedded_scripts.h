/*
 * embedded_scripts.h
 * Header file for embedded scripts
 */
#ifndef EMBEDDED_SCRIPTS_H
#define EMBEDDED_SCRIPTS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Structure to hold an embedded script */
typedef struct {
    const char *name;
    const char *content;
} EmbeddedScript;

/* Array of embedded scripts (defined in embedded_scripts.c) */
extern const EmbeddedScript embedded_scripts[];

/* Function to get an embedded script by name */
const char* get_embedded_script(const char* script_name);

#ifdef __cplusplus
}
#endif

#endif /* EMBEDDED_SCRIPTS_H */
