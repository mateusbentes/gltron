#include "gltron.h"
#include <string.h>

#ifdef ANDROID
#include <GLES2/gl2.h>
#include "shaders.h"
#else
#include <GL/gl.h>
#include <GL/freeglut.h>  // For GLUT functions
#endif

#define MENU_BUFSIZE 100

Menu *pCurrent;

// Forward declarations
static void getNextLine(char *buf, int bufsize, FILE* f);
static Menu* loadMenu(FILE* f, char* buf, Menu* parent, int level);

void changeAction(char *name) {
  printf("changeAction called with: %s\n", name);  // Debug output

#ifdef SOUND
  if(strstr(name, "audio") == name) {
    game->settings->playMusic = !game->settings->playMusic;
    if (game->settings->playSound && game->settings->playMusic) {
      playSound();
    } else {
      stopSound();
    }
  }
  if(strstr(name, "playMusic") == name) {
    game->settings->playMusic = !game->settings->playMusic;
    if (game->settings->playSound && game->settings->playMusic) {
      playSound();
    } else {
      stopSound();
    }
  }
  if(strstr(name, "playSound") == name) {
    game->settings->playSound = !game->settings->playSound;
    if (game->settings->playSound && game->settings->playMusic) {
      playSound();
    } else {
      stopSound();
    }
  }
  if(strstr(name, "menu_highlight") == name || strstr(name, "highlight") == name) {
    playSampleEffect(highlight_sfx);
  }
  if(strstr(name, "menu_action") == name || strstr(name, "action") == name) {
    playSampleEffect(action_sfx);
  }
#endif
  if(strstr(name, "resetScores") == name)
    resetScores();
  if(strstr(name, "ai_player") == name) {
    int c;
    int *v;

    /* printf("changing AI status\n"); */
    c = name[9] - '0';
    v = getVi(name);
    game->player[c - 1].ai->active = *v;
    /* printf("changed AI status for player %c\n", c + '0'); */
  }
}

void menuAction(Menu *activated) {
  int x, y;
  char c;
  int *piValue;
  if(activated->nEntries > 0) {
    pCurrent = activated;
    pCurrent->iHighlight = 0;
  } else {
    switch(activated->szName[1]) { /* second char */
    case 'q': saveSettings(); exit(0); break;
    case 'r':
      /* Ensure any pending display changes are applied before starting */
      requestDisplayApply();
      initData();
      switchCallbacks(&pauseCallbacks);
      /* Apply immediately to avoid wrong size at game start */
      applyDisplaySettingsDeferred();
      break;
    case 'v':
      sscanf(activated->szName, "%cv%dx%d ", &c, &x, &y);
      game->settings->width = x;
      game->settings->height = y;

      /* ensure our window is current before manipulating it */
      if (game && game->screen && game->screen->win_id > 0) {
#ifndef ANDROID
        glutSetWindow(game->screen->win_id);
#endif
      }

      /* defer resolution apply to idle to avoid GLUT window state issues */
      updateCallbacks();
      changeDisplay();

      /* persist new resolution immediately */
      saveSettings();
      requestDisplayApply();
      printf("Resolution set to %dx%d (apply deferred) and saved.\n", game->settings->width, game->settings->height);
      break;
    case 't':
      piValue = getVi(activated->szName + 4);
      if(piValue != 0) {
        const char* name = activated->szName + 4;
        if (strstr(name, "input_mode") == name) {
          const char* label = "Keyboard";
          if (*piValue == 1) label = "Mouse";
          else if (*piValue == 2) label = "Touch";
          sprintf(activated->display.szCaption, activated->szCapFormat, label);
        } else if (strstr(name, "fullscreen") == name) {
          int cur = *piValue;
          int next = cur ? 0 : 1;
          *piValue = next;
          game->settings->fullscreen = next;
          requestDisplayApply();
          sprintf(activated->display.szCaption, activated->szCapFormat, next ? "on" : "off");
          saveSettings();
          printf("Fullscreen setting toggled to %s (apply deferred).\n", next ? "on" : "off");
        } else if (strstr(name, "audio") == name || strstr(name, "playMusic") == name) {
          // Toggle music on/off
          game->settings->playMusic = !game->settings->playMusic;
          if (game->settings->playSound && game->settings->playMusic) {
            playSound();
          } else {
            stopSound();
          }
          sprintf(activated->display.szCaption, activated->szCapFormat, game->settings->playMusic ? "on" : "off");
          saveSettings();
        } else if (strstr(name, "playSound") == name) {
          sprintf(activated->display.szCaption, activated->szCapFormat,*piValue ? "on" : "off");
        } else {
          sprintf(activated->display.szCaption, activated->szCapFormat, *piValue ? "on" : "off");
        }
      }
      break;
    case 'p':
      changeAction(activated->szName + 4);
    case 'c':
      chooseCallback(activated->szName + 3);
      break;
    default: printf("got action for menu %s\n", activated->szName); break;
    }
  }
}

void initMenuCaption(Menu *activated) {
  int *piValue;

  /* TODO support all kinds of types */
  switch(activated->szName[0]) {
  case 's':
    switch(activated->szName[1]) {
    case 't':
      piValue = getVi(activated->szName + 4);
      if(piValue != 0) {
        const char* name = activated->szName + 4;
        if (strstr(name, "input_mode") == name) {
          const char* label = "Keyboard";
          if (*piValue == 1) label = "Mouse";
          else if (*piValue == 2) label = "Touch";
          sprintf(activated->display.szCaption, activated->szCapFormat, label);
        } else if (strstr(name, "fullscreen") == name) {
          int cur = *piValue;
          int next = cur ? 0 : 1;
          *piValue = next;
          game->settings->fullscreen = next;
          requestDisplayApply();
          sprintf(activated->display.szCaption, activated->szCapFormat, next ? "on" : "off");
          saveSettings();
          printf("Fullscreen setting toggled to %s (apply deferred).\n", next ? "on" : "off");
        } else if (strstr(name, "audio") == name || strstr(name, "playMusic") == name) {
          // Toggle music on/off
          game->settings->playMusic = !game->settings->playMusic;
          if (game->settings->playSound && game->settings->playMusic) {
            playSound();
          } else {
            stopSound();
          }
          sprintf(activated->display.szCaption, activated->szCapFormat, game->settings->playMusic ? "on" : "off");
          saveSettings();
        } else if (strstr(name, "playSound") == name) {
          sprintf(activated->display.szCaption, activated->szCapFormat,*piValue ? "on" : "off");
        } else {
          sprintf(activated->display.szCaption, activated->szCapFormat, *piValue ? "on" : "off");
        }
      }
      break;
    }
    break;

  default:
    sprintf(activated->display.szCaption, "%s", activated->szCapFormat);
  }
}

void getNextLine(char *buf, int bufsize, FILE* f) {
  fgets(buf, bufsize, f);
  while((buf[0] == '\n' || buf[0] == '#') && /* ignore empty lines, comments */
	fgets(buf, bufsize, f));
}

Menu* loadMenu(FILE* f, char* buf, Menu* parent, int level) {
  Menu* m;
  int i;


  if(level > 4) {
    printf("recursing level > 4 - aborting\n");
    exit(1);
  }

  m = (Menu*) malloc(sizeof(Menu));
  m->parent = parent;
  getNextLine(buf, MENU_BUFSIZE, f);
  sscanf(buf, "%d ", &(m->nEntries));

  getNextLine(buf, MENU_BUFSIZE, f);
  buf[31] = 0; /* enforce menu name limit; */
  sprintf(m->szName, "%s", buf);
  if(*(m->szName + strlen(m->szName) - 1) == '\n')
    *(m->szName + strlen(m->szName) - 1) = 0;
  

  getNextLine(buf, MENU_BUFSIZE, f);
  buf[31] = 0; /* enforce menu caption limit; */
  sprintf(m->szCapFormat, "%s", buf);
  /* remove newline */
  for(i = 0; *(m->szCapFormat + i) != 0; i++)
    if (*(m->szCapFormat + i) == '\n') {
      *(m->szCapFormat + i) = 0;
      break;
    }

  initMenuCaption(m);
	
  /* printf("menu '%s': %d entries\n", m->szName, m->nEntries); */
  if(m->nEntries > 0) {
    m->pEntries = malloc(sizeof(Menu*) * m->nEntries);
    for(i = 0; i < m->nEntries; i++) {
      /* printf("loading menu number %d\n", i); */
      if(i > 10) {
	printf("item limit reached - aborting\n");
	exit(1);
      }
      *(m->pEntries + i) = loadMenu(f, buf, m, level + 1);
    }
  }

  return m;
}

Menu** loadMenuFile(char *filename) {
  char buf[MENU_BUFSIZE];
  FILE* f;
  Menu* m;
  Menu** list = NULL;
  int nMenus;
  int i, j;
  node *head;
  node *t;
  node *z;
  int sp = 0;

  {
    char *resolved = getFullPath(filename);
    if(!resolved) return 0;
    char *dup = strdup(resolved);
    if(!dup) return 0;
    f = fopen(dup, "rb");
    if(f == NULL) { free(dup); return 0; }
    free(dup);
  }
  /* read count of Menus */
  getNextLine(buf, MENU_BUFSIZE, f);
  sscanf(buf, "%d ", &nMenus);
  if(nMenus <= 0) { fclose(f); return 0; }

  /* allocate space for data structures */
  list = (Menu**) malloc(sizeof(Menu*) * nMenus);

  /* load data */
  for(i = 0; i < nMenus; i++) {
    /* printf("loading menu set %d\n", i); */
    if(i > 10) { fclose(f); exit(1); }
    *(list + i) = loadMenu(f, buf, NULL, 0);
  }
  fclose(f);

  /* traverse Menu Tree and set Menu Color defaults */
  z = (node*) malloc(sizeof(node));
  z->next = z;
  head = (node*) malloc(sizeof(node));
  head->next = z;
  
  for(i = 0; i < nMenus; i++) {
    t = (node*) malloc(sizeof(node));
    t->data = *(list + i);
    t->next = head->next;
    head->next = t;
    sp++;
    while(head->next != z) {
      t = head->next;
      head->next = t->next;
      m = (Menu*) t->data;
      free(t);

      m->display.fgColor[0] = 0.0;
      m->display.fgColor[1] = 0.0;
      m->display.fgColor[2] = 0.0;
      m->display.fgColor[3] = 1.0;
      m->display.hlColor[0] = 255.0 / 255.0;
      m->display.hlColor[1] = 20.0 / 255.0;
      m->display.hlColor[2] = 20.0 / 255.0;
      m->display.hlColor[3] = 1.0;

      for(j = 0; j < m->nEntries; j++) {
        t = (node*) malloc(sizeof(node));
        t->data = *(m->pEntries + j);
        t->next = head->next;
        head->next = t;
      }
    }
  }
  return list;
}

void drawMenu(gDisplay *d) {
  /* draw Menu pCurrent */
  int i;
  int x, y, size, lineheight;

  // Setup 2D projection
#ifdef ANDROID
  // Android/GLES2: use basic shader with bottom-left origin 2D projection in pixels
  {
    GLuint prog = shader_get_basic();
    if (prog) {
      useShaderProgram(prog);
      float wvp = (float)d->vp_w;
      float hvp = (float)d->vp_h;
      GLfloat proj2D[16] = {
        2.0f / wvp, 0.0f,       0.0f, 0.0f,
        0.0f,       2.0f / hvp, 0.0f, 0.0f,
        0.0f,       0.0f,       1.0f, 0.0f,
        -1.0f,     -1.0f,       0.0f, 1.0f
      };
      GLfloat I[16] = {
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1
      };
      setProjectionMatrix(prog, (float*)proj2D);
      setViewMatrix(prog, (float*)I);
      setModelMatrix(prog, (float*)I);
      setTexture(prog, 0);
    }
  }
#else
  rasonly(d);
#endif

  x = d->vp_w / 6;
  size = d->vp_w / 32;
  if (size < 14) size = 14;
  y = 2 * d->vp_h / 3;
  lineheight = size * 2;

  // Hard fallback: if no menu loaded, display a simple message
  if (!pMenuList || !pCurrent) {
#ifdef ANDROID
    GLuint prog = shader_get_basic();
    if (prog) {
      useShaderProgram(prog);
      setColor(prog, 1.0f, 1.0f, 1.0f, 1.0f);
    }
#endif
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#ifndef ANDROID
    rasonly(d);
#endif
    drawText(x, y, size, "Menu failed to load");
    return;
  }

  /* draw the entries */
  for(i = 0; i < pCurrent->nEntries; i++) {
#ifdef ANDROID
    // Android GLES2 path: bind basic shader, set color via uniform, draw text
    {
      GLuint prog = shader_get_basic();
      if (prog) {
        useShaderProgram(prog);
        if(i == pCurrent->iHighlight) {
          setColor(prog, pCurrent->display.hlColor[0],
                   pCurrent->display.hlColor[1],
                   pCurrent->display.hlColor[2],
                   pCurrent->display.hlColor[3]);
        } else {
          setColor(prog, pCurrent->display.fgColor[0],
                   pCurrent->display.fgColor[1],
                   pCurrent->display.fgColor[2],
                   pCurrent->display.fgColor[3]);
        }
      }
    }
#else
    // Desktop OpenGL path unchanged
    if(i == pCurrent->iHighlight)
      glColor4fv(pCurrent->display.hlColor);
    else
      glColor4fv(pCurrent->display.fgColor);
#endif
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#ifndef ANDROID
    rasonly(d);
#endif
    drawText(x, y, size,
         ((Menu*)*(pCurrent->pEntries + i))->display.szCaption);
    y -= lineheight;
  }

  /* draw Back button in bottom-right */
  {
    const char* back = "Back";
    int bx = d->vp_w - (int)(size * 4);
    int by = (int)(size * 1.2f);
#ifdef ANDROID
    {
      GLuint prog = shader_get_basic();
      if (prog) {
        useShaderProgram(prog);
        setColor(prog, pCurrent->display.fgColor[0],
                 pCurrent->display.fgColor[1],
                 pCurrent->display.fgColor[2],
                 pCurrent->display.fgColor[3]);
        // Projection already set at top of drawMenu
      }
    }
#else
    glColor4fv(pCurrent->display.fgColor);
#endif
    drawText(bx, by, size, (char*)back);
  }
}

// New: Load menu from in-memory buffer (no filesystem required)
Menu** loadMenuFromBuffer(const char* buffer) {
  if (!buffer) return 0;
  // Normalize buffer: strip UTF-8 BOM and convert CRLF to LF in a temp allocation
  size_t inlen = strlen(buffer);
  const unsigned char* ub = (const unsigned char*)buffer;
  size_t start = 0;
  if (inlen >= 3 && ub[0] == 0xEF && ub[1] == 0xBB && ub[2] == 0xBF) start = 3; // skip BOM
  char* norm = (char*)malloc(inlen - start + 1);
  if (!norm) return 0;
  size_t w = 0;
  for (size_t r = start; r < inlen; ++r) {
    if (buffer[r] == '\r') continue;
    norm[w++] = buffer[r];
  }
  norm[w] = '\0';
  // Write buffer to a temporary in-memory FILE* when available, else to a temp file
#if defined(__ANDROID_API__) && (__ANDROID_API__ >= 23)
  // Try fmemopen when available
  FILE* f = fmemopen((void*)norm, strlen(norm), "rb");
  if (!f) { free(norm); return 0; }
#else
  // Portable fallback: write to a temporary file in internal storage
  char tmpPath[512];
  snprintf(tmpPath, sizeof(tmpPath), "/data/local/tmp/gltron_menu_buf_%ld.txt", (long)getpid());
  FILE* tf = fopen(tmpPath, "wb");
  if (!tf) { free(norm); return 0; }
  fwrite(norm, 1, strlen(norm), tf);
  fclose(tf);
  FILE* f = fopen(tmpPath, "rb");
  if (!f) { remove(tmpPath); free(norm); return 0; }
#endif

  char buf[MENU_BUFSIZE];
  Menu* m;
  Menu** list = NULL;
  int nMenus;
  int i, j;
  node *head;
  node *t;
  node *z;
  int sp = 0;

  // read count of Menus
  getNextLine(buf, MENU_BUFSIZE, f);
  if (sscanf(buf, "%d ", &nMenus) != 1 || nMenus <= 0) {
#if !(defined(__ANDROID_API__))
    fclose(f); remove(tmpPath);
#else
    fclose(f);
#endif
    return 0;
  }

  list = (Menu**) malloc(sizeof(Menu*) * nMenus);
  for(i = 0; i < nMenus; i++) {
    if(i > 10) {
#if !(defined(__ANDROID_API__))
      fclose(f); remove(tmpPath);
#else
      fclose(f);
#endif
      exit(1);
    }
    *(list + i) = loadMenu(f, buf, NULL, 0);
  }
  fclose(f);
#if !(defined(__ANDROID_API__))
  remove(tmpPath);
#endif

  // Traverse to set colors
  z = (node*) malloc(sizeof(node));
  z->next = z;
  head = (node*) malloc(sizeof(node));
  head->next = z;

  for(i = 0; i < nMenus; i++) {
    t = (node*) malloc(sizeof(node));
    t->data = *(list + i);
    t->next = head->next;
    head->next = t;
    sp++;
    while(head->next != z) {
      t = head->next;
      head->next = t->next;
      m = (Menu*) t->data;
      free(t);
      m->display.fgColor[0] = 0.0;
      m->display.fgColor[1] = 0.0;
      m->display.fgColor[2] = 0.0;
      m->display.fgColor[3] = 1.0;
      m->display.hlColor[0] = 255.0 / 255.0;
      m->display.hlColor[1] = 20.0 / 255.0;
      m->display.hlColor[2] = 20.0 / 255.0;
      m->display.hlColor[3] = 1.0;
      for(j = 0; j < m->nEntries; j++) {
        t = (node*) malloc(sizeof(node));
        t->data = *(m->pEntries + j);
        t->next = head->next;
        head->next = t;
      }
    }
  }
  return list;
}









