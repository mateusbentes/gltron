/* 
 * Header file for embedded scripts
 */
#ifndef EMBEDDED_SCRIPTS_H
#define EMBEDDED_SCRIPTS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Function to get an embedded script by name */
const char* get_embedded_script(const char *name);

#ifdef __cplusplus
}
#endif

#endif /* EMBEDDED_SCRIPTS_H */
