#include "base/nebu_debug_memory.h"
#include "scripting/nebu_scripting.h"
#include "base/sdl_compat.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "base/nebu_assert.h"

// static lua_State *L;
lua_State *L;
FILE* scripting_debug = NULL;
extern void init_c_interface(lua_State *L);

/* Register SDL2 key constants with their SDL 1.2 names */
void scripting_RegisterSDL2Compat(lua_State *L) {
    /* Register SDL2 key constants with their SDL 1.2 names */
    lua_pushnumber(L, SDLK_UP); lua_setglobal(L, "SDLK_UP");
    lua_pushnumber(L, SDLK_DOWN); lua_setglobal(L, "SDLK_DOWN");
    lua_pushnumber(L, SDLK_LEFT); lua_setglobal(L, "SDLK_LEFT");
    lua_pushnumber(L, SDLK_RIGHT); lua_setglobal(L, "SDLK_RIGHT");
    lua_pushnumber(L, SDLK_RETURN); lua_setglobal(L, "SDLK_RETURN");
    lua_pushnumber(L, SDLK_ESCAPE); lua_setglobal(L, "SDLK_ESCAPE");
    lua_pushnumber(L, SDLK_SPACE); lua_setglobal(L, "SDLK_SPACE");
    
    /* Register keypad keys */
    lua_pushnumber(L, SDLK_KP_0); lua_setglobal(L, "SDLK_KP0");
    lua_pushnumber(L, SDLK_KP_1); lua_setglobal(L, "SDLK_KP1");
    lua_pushnumber(L, SDLK_KP_2); lua_setglobal(L, "SDLK_KP2");
    lua_pushnumber(L, SDLK_KP_3); lua_setglobal(L, "SDLK_KP3");
    lua_pushnumber(L, SDLK_KP_4); lua_setglobal(L, "SDLK_KP4");
    lua_pushnumber(L, SDLK_KP_5); lua_setglobal(L, "SDLK_KP5");
    lua_pushnumber(L, SDLK_KP_6); lua_setglobal(L, "SDLK_KP6");
    lua_pushnumber(L, SDLK_KP_7); lua_setglobal(L, "SDLK_KP7");
    lua_pushnumber(L, SDLK_KP_8); lua_setglobal(L, "SDLK_KP8");
    lua_pushnumber(L, SDLK_KP_9); lua_setglobal(L, "SDLK_KP9");
    
    /* Register function keys */
    lua_pushnumber(L, SDLK_F1); lua_setglobal(L, "SDLK_F1");
    lua_pushnumber(L, SDLK_F2); lua_setglobal(L, "SDLK_F2");
    lua_pushnumber(L, SDLK_F3); lua_setglobal(L, "SDLK_F3");
    lua_pushnumber(L, SDLK_F4); lua_setglobal(L, "SDLK_F4");
    lua_pushnumber(L, SDLK_F5); lua_setglobal(L, "SDLK_F5");
    lua_pushnumber(L, SDLK_F6); lua_setglobal(L, "SDLK_F6");
    lua_pushnumber(L, SDLK_F7); lua_setglobal(L, "SDLK_F7");
    lua_pushnumber(L, SDLK_F8); lua_setglobal(L, "SDLK_F8");
    lua_pushnumber(L, SDLK_F9); lua_setglobal(L, "SDLK_F9");
    lua_pushnumber(L, SDLK_F10); lua_setglobal(L, "SDLK_F10");
    lua_pushnumber(L, SDLK_F11); lua_setglobal(L, "SDLK_F11");
    lua_pushnumber(L, SDLK_F12); lua_setglobal(L, "SDLK_F12");
    
    /* Register modifier keys */
    lua_pushnumber(L, SDLK_LSHIFT); lua_setglobal(L, "SDLK_LSHIFT");
    lua_pushnumber(L, SDLK_RSHIFT); lua_setglobal(L, "SDLK_RSHIFT");
    lua_pushnumber(L, SDLK_LCTRL); lua_setglobal(L, "SDLK_LCTRL");
    lua_pushnumber(L, SDLK_RCTRL); lua_setglobal(L, "SDLK_RCTRL");
    lua_pushnumber(L, SDLK_LALT); lua_setglobal(L, "SDLK_LALT");
    lua_pushnumber(L, SDLK_RALT); lua_setglobal(L, "SDLK_RALT");
    
    /* Register SDL event types */
    lua_pushnumber(L, SDL_KEYDOWN); lua_setglobal(L, "SDL_KEYDOWN");
    lua_pushnumber(L, SDL_KEYUP); lua_setglobal(L, "SDL_KEYUP");
    lua_pushnumber(L, SDL_MOUSEMOTION); lua_setglobal(L, "SDL_MOUSEMOTION");
    lua_pushnumber(L, SDL_MOUSEBUTTONDOWN); lua_setglobal(L, "SDL_MOUSEBUTTONDOWN");
    lua_pushnumber(L, SDL_MOUSEBUTTONUP); lua_setglobal(L, "SDL_MOUSEBUTTONUP");
    lua_pushnumber(L, SDL_QUIT); lua_setglobal(L, "SDL_QUIT");
    
    /* Register SDL key modifiers */
    lua_pushnumber(L, KMOD_NONE); lua_setglobal(L, "KMOD_NONE");
    lua_pushnumber(L, KMOD_LSHIFT); lua_setglobal(L, "KMOD_LSHIFT");
    lua_pushnumber(L, KMOD_RSHIFT); lua_setglobal(L, "KMOD_RSHIFT");
    lua_pushnumber(L, KMOD_LCTRL); lua_setglobal(L, "KMOD_LCTRL");
    lua_pushnumber(L, KMOD_RCTRL); lua_setglobal(L, "KMOD_RCTRL");
    lua_pushnumber(L, KMOD_LALT); lua_setglobal(L, "KMOD_LALT");
    lua_pushnumber(L, KMOD_RALT); lua_setglobal(L, "KMOD_RALT");
    lua_pushnumber(L, KMOD_SHIFT); lua_setglobal(L, "KMOD_SHIFT");
    lua_pushnumber(L, KMOD_CTRL); lua_setglobal(L, "KMOD_CTRL");
    lua_pushnumber(L, KMOD_ALT); lua_setglobal(L, "KMOD_ALT");
    
    /* Register SDL mouse buttons */
    lua_pushnumber(L, SDL_BUTTON_LEFT); lua_setglobal(L, "SDL_BUTTON_LEFT");
    lua_pushnumber(L, SDL_BUTTON_MIDDLE); lua_setglobal(L, "SDL_BUTTON_MIDDLE");
    lua_pushnumber(L, SDL_BUTTON_RIGHT); lua_setglobal(L, "SDL_BUTTON_RIGHT");
    
    /* Register SDL video flags */
    lua_pushnumber(L, SDL_WINDOW_FULLSCREEN); lua_setglobal(L, "SDL_FULLSCREEN");
    lua_pushnumber(L, SDL_WINDOW_OPENGL); lua_setglobal(L, "SDL_OPENGL");
    lua_pushnumber(L, SDL_WINDOW_RESIZABLE); lua_setglobal(L, "SDL_RESIZABLE");
}

void scripting_Init(int flags) {
  L = lua_open();
  luaopen_base(L);
  luaopen_table(L);
  luaopen_string(L);
  luaopen_math(L);
  luaopen_io(L);

  /* Register SDL2 compatibility functions */
  scripting_RegisterSDL2Compat(L);

  if(flags | NEBU_SCRIPTING_DEBUG)
  {
	  scripting_debug = fopen("scripting.txt", "w");
  }

  // init_c_interface(L);
}

void scripting_Quit() {
  lua_close(L);
}


void showStack() {
	int i;
	printf("dumping stack with %d elements\n", lua_gettop(L));
	for(i = 0; i < lua_gettop(L); i++) {
		int type = lua_type(L, - (i+1));
		switch(type) {
		case LUA_TNIL: printf("nil\n"); break;
		case LUA_TNUMBER: printf("number\n"); break;
		case LUA_TSTRING: printf("string\n"); break;
		case LUA_TTABLE: printf("table\n"); break;
		case LUA_TFUNCTION: printf("function\n"); break;
		case LUA_TUSERDATA: printf("userdata\n"); break;
		}
	}
}

int scripting_IsNil() {
	return lua_isnil(L, -1);
}

int getGlobal(const char *s, va_list ap) {
	int top = lua_gettop(L);
	int count = 0;
	while(s) {
		lua_pushstring(L, s);
		/* TODO: add error checking */
		lua_gettable(L, -2);
		count++;
		s = va_arg(ap, char *);
	}
	lua_insert(L, top); /* move result to bottom */
	lua_pop(L, count); /* restore stack */
	return 0;
}

int scripting_GetValue(const char *name) {
	int top = lua_gettop(L);
	lua_pushstring(L, name);
	lua_gettable(L, -2);
	nebu_assert( lua_gettop(L) == top + 1 );

	return 0;
}

int scripting_GetGlobal(const char *global, const char *s, ...) {
	lua_getglobal(L, global);
	if(s) {
		va_list ap;
		va_start(ap, s);
		getGlobal(s, ap);
		va_end(ap);
	}
	return 0;
}	

int scripting_SetFloat(float f, const char *name, const char *global, const char *s, ...) {
	va_list ap;
	
	if(global == NULL) {
		lua_pushnumber(L, f);
		lua_setglobal(L, global);
		return 0;
	}
	
	lua_getglobal(L, global);
	
	if(s) {
		va_start(ap, s);
		getGlobal(s, ap);
		va_end(ap);
	}
	
	lua_pushstring(L, name);
	lua_pushnumber(L, f);
	lua_settable(L, -3);
	lua_pop(L, 1);

	return 0;
}

int scripting_GetFloatResult(float *f)
{
	if(lua_isnumber(L, -1))
	{
		*f = (float) lua_tonumber(L, -1);
		lua_pop(L, 1); /* restore stack */
		return 0;
	}
	else
	{
		showStack();
		return 1;
	}
}  

int scripting_GetIntegerResult(int *i)
{
	if(lua_isnumber(L, -1))
	{
		*i = (int)lua_tonumber(L, -1);
		lua_pop(L, 1); /* restore stack */
		return 0;
	}
	else
	{
		showStack();
		return 1;
	}
}  

void scripting_GetFloatArrayResult(float *f, int n) {
  int i;
	
  for(i = 0; i < n; i++) {
    lua_rawgeti(L, -1, i + 1);
    if(lua_isnumber(L, -1)) {
      *(f + i) = (float)lua_tonumber(L, -1);
    } else {
      fprintf(stderr, "element %d is not number!\n", i);
    }
    lua_pop(L, 1); /* remove number from stack */
  }

	lua_pop(L, 1); /* remove table from stack */
}

/*!
	allocates a string, copies the result from the stack into it
	and pops the value from the stack
*/
int scripting_GetStringResult(scripting_StringResult *s)
{
  int status;

  if(lua_isstring(L, -1)) {
    int size;
    status = 0;
    size = lua_strlen(L, -1) + 1;
    *s = malloc( size );
    memcpy( *s, lua_tostring(L, -1), size );
    /* printf("allocated string '%s' of size %d\n", *s, size); */
  } else
    status = 1;

  lua_pop(L, 1);
  return status;
}

void scripting_StringResult_Free(scripting_StringResult s)
{
	free(s);
}

/*!
	copies the value from the stack into the supplied buffer
	(at most len bytes) and pops the value from the stack
*/

int scripting_CopyStringResult(char *s, int len) {
  int status;
  if(lua_isstring(L, -1)) {
    int size, copy;
    status = 0;
    size = lua_strlen(L, -1) + 1;
    if(size > len) { copy = len; status = 2; }
    else copy = size;
    memcpy( s, lua_tostring(L, -1), size );
  } else
    status = 1;

  lua_pop(L, 1);
  return status;
}    

int scripting_GetArraySize(int *i)
{
	*i = luaL_getn(L, -1);
	return 0;
}

int scripting_GetArrayIndex(int i)
{
	lua_rawgeti(L, -1, i);
	return 0;
}

int scripting_IsTable(void)
{
	return lua_istable(L, -1);
}

int scripting_Pop(void)
{
	lua_pop(L, 1);
	return 0;
}

int run(const char *command, int crlf)
{
	int status;
	if(scripting_debug)
	{
		fwrite(command, strlen(command), 1, scripting_debug);
		if(crlf)
		{
			fputc('\n', scripting_debug);
		}
		fflush(scripting_debug);
	}
	status = lua_dostring(L, command);
	nebu_assert(!status);
	return status;
}

int scripting_RunFile(const char *name) {
	int status;
	if(scripting_debug)
	{
		FILE *f;
		char buf[65536];
		int line = 0;

		f = fopen(name, "r");
		if(f)
			fprintf(scripting_debug, "-- [SCRIPTING DEBUG RUNTINE] opening '%s' succesful\n", name);
		else
			fprintf(scripting_debug, "-- [SCRIPTING DEBUG RUNTINE] opening '%s' failed\n", name);
		while(f)
		{
			char *tmp = fgets(buf, sizeof(buf), f);
			if(tmp != NULL)
				fwrite(buf, strlen(buf), 1, scripting_debug);
			else
			{
				fclose(f);
				f = NULL;
			}	
		}
		fflush(scripting_debug);
	}
	status = lua_dofile(L, name);
	nebu_assert(!status);
	return status;
}

int scripting_Run(const char *command) {
	return run(command, NEBU_SCRIPTING_CRLF);
}

int scripting_RunFormat(const char *format, ... ) {
  char buf[4096];
  va_list ap;
  va_start(ap, format);
  vsprintf(buf, format, ap);
  va_end(ap);
  return run(buf, NEBU_SCRIPTING_CRLF);
}

void scripting_RunGC() {
  lua_setgcthreshold(L, 0);
}

void Scripting_Idle() {
	scripting_RunGC();
}

void scripting_Register(const char *name, int(*func) (lua_State *L)) {
	lua_register(L, name, func);
}

void scripting_PushInteger(int iValue)
{
	lua_pushnumber(L, (float)iValue);
}

static int piStackGuard[256];
static int iStackGuardPosition = -1;

int scripting_StackGuardStart(void)
{
	if(iStackGuardPosition == 255)
	{
		nebu_assert(0);
		return -1;
	}

	iStackGuardPosition++;
	piStackGuard[iStackGuardPosition] = lua_gettop(L);

	return iStackGuardPosition;
}

void scripting_StackGuardEnd(int iPos)
{
	if(iStackGuardPosition == -1)
	{
		nebu_assert(0);
		return;
	}

	nebu_assert(iPos == iStackGuardPosition);
	nebu_assert(piStackGuard[iPos] == lua_gettop(L));
	iStackGuardPosition--;
}

int scripting_GetOptional_Float(const char *name, float *f, float fValue)
{
	int retValue = 0;
	scripting_GetValue(name);
	if(!scripting_IsNil())
	{
		retValue = scripting_GetFloatResult(f);
	}
	else
	{
		*f = fValue;
		scripting_Pop();
	}
	return retValue;
}

int scripting_GetOptional_Int(const char *name, int *i, int iValue)
{
	int retValue = 0;
	scripting_GetValue(name);
	if(!scripting_IsNil())
	{
		retValue = scripting_GetIntegerResult(i);
	}
	else
	{
		*i = iValue;
		scripting_Pop();
	}
	return retValue;
}
