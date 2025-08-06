#ifndef NEBU_SCRIPTING_H
#define NEBU_SCRIPTING_H

#ifdef __cplusplus
extern "C" {
#endif

// Scripting system stubs for Android
int scripting_RunFile(const char* filename);
int scripting_Init(void);
void scripting_Shutdown(void);
int scripting_IsEnabled(void);

#ifdef __cplusplus
}
#endif

#endif /* NEBU_SCRIPTING_H */

