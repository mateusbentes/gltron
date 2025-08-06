#include "audio/audio.h"
#include "video/video.h"
#include "video/graphics_utility.h"
#include "input/input.h"
#include "configuration/configuration.h"
#include "configuration/settings.h"
#include "filesystem/path.h"
#include "scripting/scripting.h"
#include "game/32bit_warning.h"

#include "video/nebu_2d.h"
#include "video/nebu_renderer_gl.h"
#include "video/nebu_video_system.h"
#include "input/nebu_input_system.h"
#include "base/nebu_math.h"
#include "scripting/nebu_scripting.h"
#include "filesystem/nebu_filesystem.h"

#include "base/nebu_assert.h"
#include <string.h>

#include "base/nebu_debug_memory.h"

// Define the Callbacks structure
typedef struct {
  void (*display)(void);
  void (*idle)(void);
  void (*keyboard)(int state, int key, int x, int y);
  void (*init)(void);
  void (*exit)(void);
  void (*mouse)(int buttons, int state, int x, int y);
  void (*mouseMotion)(int x, int y);
  void (*reshape)(int width, int height);
  const char *name;
} Callbacks;

// local resources
nebu_2d *pBackground = NULL;
nebu_Font *pFont = NULL;

void drawGuiMenu(Visual *d);

void drawGuiBackground(void) {
  nebu_Video_CheckErrors("gui background start");

  // rasonly(gScreen);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, pBackground->w, 0, pBackground->h, 0, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glDisable(GL_DEPTH_TEST);
  nebu_2d_Draw(pBackground);
}

void displayGui(void) {
#ifdef USE_SCRIPTING
  scripting_GetGlobal("gui_hide_background", NULL);
  if (!scripting_IsNil()) {
    scripting_Pop();
    _32bit_warning_Display();
    return;
  }
  scripting_Pop();
#endif

  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  drawGuiBackground();
  drawGuiMenu(gScreen);

  nebu_System_SwapBuffers();
}

void displayConfigure(void) {
  char message[] = "Press a key for this action!";
  drawGuiBackground();
  drawGuiMenu(gScreen);

  rasonly(gScreen);
  glColor4f(1.0, 1.0, 1.0, 1.0f);
  drawText(pFont, gScreen->vp_w / 6.0f, 20,
           gScreen->vp_w / (6.0f / 4.0f * strlen(message)), message);
  nebu_System_SwapBuffers();
}

void idleGui(void) {
  Sound_idle();
#ifdef USE_SCRIPTING
  scripting_RunGC();
#endif
  Video_Idle();
  Input_Idle();
  nebu_Time_FrameDelay(50);
  nebu_System_PostRedisplay(); /* animate menu */
}

void keyboardConfigure(int state, int key, int x, int y) {
  if (state != NEBU_INPUT_KEYSTATE_DOWN)
    return;

  if (key != 27) /* don't allow escape */
  {
    int i;
    int isReserved = 0;
    for (i = 0; i < eReservedKeys; i++) {
      if (key == ReservedKeyCodes[i]) {
        isReserved = 1;
        break;
      }
    }
    if (!isReserved) {
#ifdef USE_SCRIPTING
      scripting_RunFormat("settings.keys[ configure_player ]"
                           "[ configure_event ] = %d",
                           key);
#endif
    }
  }
  nebu_System_ExitLoop(eSRC_GUI_Prompt_Escape);
}

void keyboardGui(int state, int key, int x, int y) {
  char *pMenuName;

  if (state == NEBU_INPUT_KEYSTATE_UP)
    return;

#ifdef USE_SCRIPTING
  scripting_Run("return Menu.current");
  scripting_GetStringResult(&pMenuName);
#endif

  switch (key) {
  case 27:
#ifdef USE_SCRIPTING
    if (strcmp(pMenuName, "RootMenu")) {
      scripting_Run("MenuFunctions.GotoParent()");
    } else {
#endif
      nebu_System_ExitLoop(eSRC_GUI_Escape);
#ifdef USE_SCRIPTING
    }
#endif
    break;
  case ' ':
  case SYSTEM_KEY_RETURN:
  case SYSTEM_JOY_BUTTON_0:
  case SYSTEM_JOY_BUTTON_1:
  case SYSTEM_JOY_BUTTON_2:
  case SYSTEM_JOY_BUTTON_3:
  case SYSTEM_JOY_BUTTON_4:
  case SYSTEM_JOY_BUTTON_5:
  case SYSTEM_JOY_BUTTON_6:
  case SYSTEM_JOY_BUTTON_7:
  case SYSTEM_JOY_BUTTON_8:
  case SYSTEM_JOY_BUTTON_9:
  case SYSTEM_JOY_BUTTON_0 + SYSTEM_JOY_OFFSET:
  case SYSTEM_JOY_BUTTON_1 + SYSTEM_JOY_OFFSET:
  case SYSTEM_JOY_BUTTON_2 + SYSTEM_JOY_OFFSET:
  case SYSTEM_JOY_BUTTON_3 + SYSTEM_JOY_OFFSET:
  case SYSTEM_JOY_BUTTON_4 + SYSTEM_JOY_OFFSET:
  case SYSTEM_JOY_BUTTON_5 + SYSTEM_JOY_OFFSET:
  case SYSTEM_JOY_BUTTON_6 + SYSTEM_JOY_OFFSET:
  case SYSTEM_JOY_BUTTON_7 + SYSTEM_JOY_OFFSET:
  case SYSTEM_JOY_BUTTON_8 + SYSTEM_JOY_OFFSET:
  case SYSTEM_JOY_BUTTON_9 + SYSTEM_JOY_OFFSET:
#ifdef USE_SCRIPTING
    scripting_Run("MenuFunctions.Action()");
#endif
    break;
  case SYSTEM_KEY_UP:
  case SYSTEM_JOY_UP:
  case SYSTEM_JOY_UP + SYSTEM_JOY_OFFSET:
#ifdef USE_SCRIPTING
    scripting_Run("MenuFunctions.Previous()");
#endif
    break;
  case SYSTEM_KEY_DOWN:
  case SYSTEM_JOY_DOWN:
  case SYSTEM_JOY_DOWN + SYSTEM_JOY_OFFSET:
#ifdef USE_SCRIPTING
    scripting_Run("MenuFunctions.Next()");
#endif
    break;
  case SYSTEM_KEY_RIGHT:
  case SYSTEM_JOY_RIGHT:
  case SYSTEM_JOY_RIGHT + SYSTEM_JOY_OFFSET:
#ifdef USE_SCRIPTING
    scripting_Run("MenuFunctions.Right()");
#endif
    break;
  case SYSTEM_KEY_LEFT:
  case SYSTEM_JOY_LEFT:
  case SYSTEM_JOY_LEFT + SYSTEM_JOY_OFFSET:
#ifdef USE_SCRIPTING
    scripting_Run("MenuFunctions.Left()");
#endif
    break;
  case SYSTEM_KEY_F11:
    doBmpScreenShot(gScreen);
    break;
  case SYSTEM_KEY_F12:
    doPngScreenShot(gScreen);
    break;
  default:
    // printf("got key %d\n", key);
    break;
  }
#ifdef USE_SCRIPTING
  scripting_StringResult_Free(pMenuName);
#endif
}

void initGui(void) {
  gui_LoadResources();
  updateSettingsCache();
}

void gui_LoadResources(void) {
  char *path;

  path = getPath(PATH_DATA, "babbage.ftx");
  if (path) {
    pFont = nebu_Font_Load(path, PATH_ART);
    free(path);
  } else {
    fprintf(stderr, "[fata]: can't find babbage.ftx!\n");
    // installation corrupt
    nebu_assert(0);
    exit(1);
  }

  path = nebu_FS_GetPath_WithFilename(PATH_ART, "gui.png");
  if (path) {
    pBackground = nebu_2d_LoadPNG(path, 0);
    free(path);
  } else {
    fprintf(stderr, "[fata]: can't gui.png!\n");
    // installation corrupt
    nebu_assert(0);
    exit(1);
  }
}

void gui_ReleaseResources() {
  if (pFont) {
    nebu_Font_Free(pFont);
    pFont = NULL;
  }
  if (pBackground) {
    nebu_2d_Free(pBackground);
    pBackground = NULL;
  }
}

void exitGui(void) {
  gui_ReleaseResources();
  updateSettingsCache(); // GUI can change settings
}

void guiMouse(int buttons, int state, int x, int y) {
  fprintf(stderr, "Mouse buttons: %d, State %d, Position (%d, %d)\n",
          buttons, state, x, y);

  /* fprintf(stderr, "testing for state == %d\n", SYSTEM_MOUSEPRESSED); */
}

void guiMouseMotion(int mx, int my) {
  /* TODO: add mouse cursor, highlighted areas, etc. */
}

int guiMainLoop(void) {
  int status = 0;

  printf("[game] Running GUI callback\n");

  // Process input events
  status = nebu_System_MainLoop();
  if (status == 0) {
    return 0; // Quit
  }

  // Update game state
  updateGame();

  // Display the game
  displayGame();

  return 1; // Continue
}

void drawGuiMenu(Visual *d) {
  /* draw Menu pCurrent */

  int i;
  int x, y, size, lineheight;
  int hsize, vsize;
  int max_label = 0;
  int max_data = 0;
  int nEntries;
  char pMenuName[200];
  int iActiveItem;

  rasonly(d);

#define MENU_TEXT_START_X 0.08
#define MENU_TEXT_START_Y 0.40

#define MENU_WIDTH 0.80
#define MENU_HEIGHT 0.40

#define MENU_TEXT_LINEHEIGHT 1.5

  x = (int)(d->vp_w * MENU_TEXT_START_X);
  y = (int)(d->vp_h * MENU_TEXT_START_Y);

  /* obtain menu name */
#ifdef USE_SCRIPTING
  scripting_Run("return Menu.current");
  scripting_CopyStringResult(pMenuName, 200);
  /* obtain some information about the active menu */
  scripting_RunFormat("return table.getn( Menu.%s.items )", pMenuName);
  scripting_GetIntegerResult(&nEntries);
#endif

  /* new stuff: calculate menu dimensions */
  for (i = 0; i < nEntries; i++) {
    int len_label = 0;
    int len_data = 0;

#ifdef USE_SCRIPTING
    scripting_RunFormat("return string.len( Menu[Menu.%s.items[%d]].caption )",
                        pMenuName, i + 1);
    scripting_GetIntegerResult(&len_label);
    len_label += 2; /* add ': ' */
    scripting_RunFormat("return GetMenuValueWidth( Menu.%s.items[%d] )",
                        pMenuName, i + 1);
    scripting_GetIntegerResult(&len_data);
#endif

    if (len_label > max_label)
      max_label = len_label;
    if (len_data > max_data)
      max_data = len_data;
  }

  /* adjust size so menu fits into MENU_WIDTH/HEIGHT */

  hsize = (int)((float)d->vp_w * MENU_WIDTH / (float)(max_label + max_data));
  vsize = (int)((float)d->vp_h * MENU_HEIGHT /
                ((float)nEntries * MENU_TEXT_LINEHEIGHT));

  size = (hsize < vsize) ? hsize : vsize;

  lineheight = (int)((float)size * MENU_TEXT_LINEHEIGHT);

  /* printf("%d %d %d %d %d\n", x, y, size, maxw, pCurrent->nEntries); */
  /* draw the entries */

#ifdef USE_SCRIPTING
  scripting_Run("return Menu.active");
  scripting_GetIntegerResult(&iActiveItem);
#endif

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  for (i = 0; i < nEntries; i++) {
    if (i == iActiveItem - 1) {
      float color[4];
      float active1[4];
      float active2[4];
      int j;
      float t;
      int time = nebu_Time_GetElapsed() & 4095;
      t = sinf(time * M_PI / 2048.0) / 2.0f + 0.5f;

#ifdef USE_SCRIPTING
      scripting_GetGlobal("menu_item_active1", NULL);
      scripting_GetFloatArrayResult(active1, 4);
      scripting_GetGlobal("menu_item_active2", NULL);
      scripting_GetFloatArrayResult(active2, 4);
#endif

      for (j = 0; j < 4; j++) {
        color[j] = t * active1[j] + (1 - t) * active2[j];
      }
      glColor4f(color[0], color[1], color[2], color[3]);
    } else {
      float color[4];
#ifdef USE_SCRIPTING
      scripting_GetGlobal("menu_item", NULL);
      scripting_GetFloatArrayResult(color, 4);
#endif
      glColor4f(color[0], color[1], color[2], color[3]);
    }

    {
      char line_label[100];
      char line_data[100];
#ifdef USE_SCRIPTING
      scripting_RunFormat("return "
                         "GetMenuValueString( Menu.%s.items[%d] )",
                         pMenuName, i + 1);
      scripting_CopyStringResult(line_data, sizeof(line_data));

      if (line_data[0] != 0)
        scripting_RunFormat("return "
                            "Menu[Menu.%s.items[%d]].caption .. ': '",
                            pMenuName, i + 1);
      else
        scripting_RunFormat("return "
                            "Menu[Menu.%s.items[%d]].caption",
                            pMenuName, i + 1);

      scripting_CopyStringResult(line_label, sizeof(line_label));
#endif

      drawText(pFont, (float)x, (float)y, (float)size, line_label);
      drawText(pFont, (float)x + max_label * size, (float)y, (float)size,
               line_data);
    }

    /*
    if(i == pCurrent->iHighlight)
      drawSoftwareHighlight(x, y, size, ((Menu*)*(pCurrent->pEntries + i))->display.szCaption);
    */
    y -= lineheight;
  }

  glDisable(GL_BLEND);
}

Callbacks configureCallbacks = {
    displayConfigure, idleGui, keyboardConfigure, initGui, NULL /* exit */,
    NULL /* mouse button */, NULL /* mouse motion */, NULL /* reshape */, "configure"
};

Callbacks guiCallbacks = {
    displayGui, idleGui, keyboardGui, initGui, exitGui,
    guiMouse, guiMouseMotion, NULL /* reshape */, "gui"
};
