/*
 * lua_compat.h
 * Compatibility layer for different Lua versions
 */
#ifndef LUA_COMPAT_H
#define LUA_COMPAT_H

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <stdio.h>

// Define LUA_OK for Lua 5.1 compatibility
#ifndef LUA_OK
#define LUA_OK 0
#endif

// Safe wrapper for lua_checkstack
static int safe_lua_checkstack(lua_State *L, int extra) {
    if (!L) return 0;
    
    // Try to grow the stack
    int result = lua_checkstack(L, extra);
    
    // Verify the stack is valid after growing
    if (result && lua_gettop(L) < 0) {
        fprintf(stderr, "[FATAL] Lua stack corrupted after growing\n");
        return 0;
    }
    
    return result;
}

// Safe wrapper for lua_pushcfunction
static void safe_lua_pushcfunction(lua_State *L, lua_CFunction f) {
    if (!L || !f) {
        fprintf(stderr, "[FATAL] Invalid parameters for lua_pushcfunction\n");
        return;
    }
    
    lua_pushcfunction(L, f);
}

// Safe wrapper for lua_setglobal
static void safe_lua_setglobal(lua_State *L, const char *name) {
    if (!L || !name) {
        fprintf(stderr, "[FATAL] Invalid parameters for lua_setglobal\n");
        return;
    }
    
    lua_setglobal(L, name);
}

// Safe wrapper for luaL_loadbuffer
static int safe_luaL_loadbuffer(lua_State *L, const char *buff, size_t sz, const char *name) {
    if (!L || !buff || !name) {
        fprintf(stderr, "[FATAL] Invalid parameters for luaL_loadbuffer\n");
        return LUA_ERRRUN;
    }
    
    return luaL_loadbuffer(L, buff, sz, name);
}

// Safe wrapper for lua_pcall
static int safe_lua_pcall(lua_State *L, int nargs, int nresults, int errfunc) {
    if (!L) {
        fprintf(stderr, "[FATAL] Invalid Lua state for lua_pcall\n");
        return LUA_ERRRUN;
    }
    
    return lua_pcall(L, nargs, nresults, errfunc);
}

// Safe wrapper for lua_pop
static void safe_lua_pop(lua_State *L, int n) {
    if (!L) {
        fprintf(stderr, "[FATAL] Invalid Lua state for lua_pop\n");
        return;
    }
    
    if (n > 0 && lua_gettop(L) >= n) {
        lua_pop(L, n);
    } else {
        fprintf(stderr, "[WARNING] Attempted to pop %d elements from stack with only %d elements\n", 
                n, lua_gettop(L));
        // Pop all available elements if n is too large
        if (lua_gettop(L) > 0) {
            lua_pop(L, lua_gettop(L));
        }
    }
}

#endif /* LUA_COMPAT_H */
