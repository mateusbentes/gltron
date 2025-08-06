/*
** $Id: lstring.c,v 1.1 2005/01/01 21:31:48 andi75 Exp $
** String table (keeps all strings handled by Lua)
** See Copyright Notice in lua.h
*/


#include <string.h>
#include <stdio.h>  /* Added for fprintf and stderr */

#define lstring_c

#include "lua.h"

#include "lmem.h"
#include "lobject.h"
#include "lstate.h"
#include "lstring.h"
#include "lgc.h"  /* Added for isdead and changewhite */

/* Define isdead and changewhite macros if they're not defined */
#ifndef isdead
#define isdead(g,v)  ((v)->gch.marked & (1<<6))
#endif

#ifndef changewhite
#define changewhite(x) ((x)->gch.marked ^= (1<<5))
#endif

void luaS_freeall (lua_State *L) {
  lua_assert(G(L)->strt.nuse==0);
  luaM_freearray(L, G(L)->strt.hash, G(L)->strt.size, TString *);
}


void luaS_resize (lua_State *L, int newsize) {
  GCObject **newhash = luaM_newvector(L, newsize, GCObject *);
  stringtable *tb = &G(L)->strt;
  int i;
  for (i=0; i<newsize; i++) newhash[i] = NULL;
  /* rehash */
  for (i=0; i<tb->size; i++) {
    GCObject *p = tb->hash[i];
    while (p) {  /* for each node in the list */
      GCObject *next = p->gch.next;  /* save next */
      lu_hash h = gcotots(p)->tsv.hash;
      int h1 = lmod(h, newsize);  /* new position */
      lua_assert(cast(int, h%newsize) == lmod(h, newsize));
      p->gch.next = newhash[h1];  /* chain it */
      newhash[h1] = p;
      p = next;
    }
  }
  luaM_freearray(L, tb->hash, tb->size, TString *);
  tb->size = newsize;
  tb->hash = newhash;
}


static TString *newlstr (lua_State *L, const char *str, size_t l, lu_hash h) {
  TString *ts = cast(TString *, luaM_malloc(L, sizestring(l)));
  stringtable *tb;
  ts->tsv.len = l;
  ts->tsv.hash = h;
  ts->tsv.marked = 0;
  ts->tsv.tt = LUA_TSTRING;
  ts->tsv.reserved = 0;
  memcpy(ts+1, str, l*sizeof(char));
  ((char *)(ts+1))[l] = '\0';  /* ending 0 */
  tb = &G(L)->strt;
  h = lmod(h, tb->size);
  ts->tsv.next = tb->hash[h];  /* chain new entry */
  tb->hash[h] = valtogco(ts);
  tb->nuse++;
  if (tb->nuse > cast(ls_nstr, tb->size) && tb->size <= MAX_INT/2)
    luaS_resize(L, tb->size*2);  /* too crowded */
  return ts;
}


/* Define rawgco2ts if it's not defined */
#ifndef rawgco2ts
#define rawgco2ts(o) check_exp((o)->gch.tt == LUA_TSTRING, &((o)->ts))
#endif

TString *luaS_newlstr (lua_State *L, const char *str, size_t l) {
  GCObject *o;
  unsigned int h = cast(unsigned int, l);  /* seed */
  size_t step = (l>>5)+1;  /* if string is too long, don't hash all its chars */
  size_t l1;
  
  fprintf(stderr, "[DEBUG] luaS_newlstr called with string: %s, length: %zu\n", str, l);
  
  for (l1=l; l1>=step; l1-=step)  /* compute hash */
    h = h ^ ((h<<5)+(h>>2)+cast(unsigned char, str[l1-1]));
  for (o = G(L)->strt.hash[lmod(h, G(L)->strt.size)];
       o != NULL;
       o = o->gch.next) {
    TString *ts = rawgco2ts(o);
    if (ts->tsv.len == l && (memcmp(str, getstr(ts), l) == 0)) {
      /* string may be dead */
      if (isdead(G(L), o)) changewhite(o);
      return ts;
    }
  }
  
  fprintf(stderr, "[DEBUG] Creating new string\n");
  
  return newlstr(L, str, l, h);  /* not found */
}


Udata *luaS_newudata (lua_State *L, size_t s) {
  Udata *u;
  u = cast(Udata *, luaM_malloc(L, sizeudata(s)));
  u->uv.marked = (1<<1);  /* is not finalized */
  u->uv.tt = LUA_TUSERDATA;
  u->uv.len = s;
  u->uv.metatable = hvalue(defaultmeta(L));
  /* chain it on udata list */
  u->uv.next = G(L)->rootudata;
  G(L)->rootudata = valtogco(u);
  return u;
}
