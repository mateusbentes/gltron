#ifndef MENUS
#define MENUS

typedef struct {
  /* fonttex *font; */
  float fgColor[4]; /* entries */
  float hlColor[4]; /* the highlighted one */

  char szCaption[32];
} mDisplay;

typedef struct Menu {
  int nEntries;
  int iHighlight;
  mDisplay display;
  char szName[32];
  char szCapFormat[32];
  struct Menu** pEntries;
  struct Menu* parent;
  void* param; /* reserved to bind parameters at runtime */
} Menu;

typedef struct {
  void* data;
  void* next;
} node;

#endif





