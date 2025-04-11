#ifndef SCRIPTING_H
#define SCRIPTING_H

#include "lua.h"

/* Initialize the scripting system */
void scripting_Init(int debug);

/* Shut down the scripting system */
void scripting_Quit(void);

/* Run a Lua script from a file */
void scripting_RunFile(const char *name);

/* Run a Lua script from a string */
void scripting_RunString(const char *script);

/* Run a Lua script */
int scripting_Run(const char *command);

/* Register a C function with Lua */
void scripting_Register(const char *name, int (*f)(lua_State *L));

/* Get the Lua state */
lua_State* scripting_GetLuaState(void);

/* Run a script from a path and name */
void runScript(int path, const char *name);

#endif /* SCRIPTING_H */
