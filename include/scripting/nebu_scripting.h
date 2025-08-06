#ifndef SCRIPTING_NEBU_SCRIPTING_H
#define SCRIPTING_NEBU_SCRIPTING_H

// Android stub for scripting/nebu_scripting.h
// Simplified scripting interface for Android

typedef struct {
    int initialized;
} ScriptingSystem;

// Basic scripting functions
int scripting_Init(void);
void scripting_Shutdown(void);
int scripting_RunScript(const char* script);
int scripting_LoadFile(const char* filename);
void scripting_Run(const char* script);
void scripting_RunFormat(const char* format, ...);
int scripting_GetGlobal(const char* name, ...);
int scripting_GetFloatResult(float* result);
int scripting_IsNil(void);
void scripting_Pop(void);
void scripting_SetFloat(float value, const char* name, ...);
int scripting_GetFloatArrayResult(float* result, int count);

// Settings integration
void scripting_RegisterSettings(void);

#endif // SCRIPTING_NEBU_SCRIPTING_H

