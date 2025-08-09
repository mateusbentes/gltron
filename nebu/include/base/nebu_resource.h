#ifndef NEBU_RESOURCE_H
#define NEBU_RESOURCE_H
#include "game/resource.h" // or forward declare resource_token and EResourceType

void resource_RegisterHandler(int type, void* (*get)(char*,void*), void (*release)(void *));
void resource_UnregisterHandler(int type);

void resource_Release(int token);
void resource_ReleaseAll(void);
void resource_ReleaseType(int type);
void resource_FreeAll(void);

void resource_Free(resource_token *token);
resource_token *resource_GetToken(const char *filename, EResourceType type);
int resource_GetTokenMeta(char *filename, int type, void *metadata, int metasize);

#endif