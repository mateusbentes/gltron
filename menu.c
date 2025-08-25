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
  if(nMenus <= 0) return 0;

  /* allocate space for data structures */
  list = (Menu**) malloc(sizeof(Menu*) * nMenus);

  /* load data */
  for(i = 0; i < nMenus; i++) {
    /* printf("loading menu set %d\n", i); */
    if(i > 10) exit(1);
    *(list + i) = loadMenu(f, buf, NULL, 0);
  }

  /* TODO(3): now since I eliminated the need for cx/cy, why */
  /* do I need to traverse the Menu Tree? Just to set the colors??? */

  /* traverse Menu Tree and set Menu Color to some boring default */
  /* printf("finished parsing file - now traversing menus\n"); */
  /* setup stack */
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
      /* printf("stack count: %d\n", --sp); */
      /* printf("visiting %s\n", m->szName); */
      /* visit m */

      /* TODO(0): put the color defaults somewhere else */

      m->display.fgColor[0] = 0.0;
      m->display.fgColor[1] = 0.0;
      m->display.fgColor[2] = 0.0;
      m->display.fgColor[3] = 1.0;
      m->display.hlColor[0] = 255.0 / 255.0;
      m->display.hlColor[1] = 20.0 / 255.0;
      m->display.hlColor[2] = 20.0 / 255.0;
      m->display.hlColor[3] = 1.0;

      /* push all of m's submenus */
      for(j = 0; j < m->nEntries; j++) {
	t = (node*) malloc(sizeof(node));
	t->data = *(m->pEntries + j);
	t->next = head->next;
	head->next = t;
	/* printf("pushing %s\n", ((Menu*)t->data)->szName); */
	/* printf("stack count: %d\n", ++sp); */
	
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
  rasonly(d);

  x = d->vp_w / 6;
  size = d->vp_w / 32;
  y = 2 * d->vp_h / 3;
  lineheight = size * 2;

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
    rasonly(d);
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
      }
    }
#else
    glColor4fv(pCurrent->display.fgColor);
#endif
    drawText(bx, by, size, (char*)back);
  }
}


















