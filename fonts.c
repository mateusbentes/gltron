#include "gltron.h"

void initFonts() {
  char *path;

  if(ftx != NULL) ftxUnloadFont(ftx);
  path = getFullPath("xenotron.ftx");
  if(path != 0) {
    ftx = ftxLoadFont(path);
  
    if(ftx == NULL) {
      // fprintf(stderr, "fatal: no Fonts available - %s, %s\n",
      //       path, txfErrorString());
      exit(1);
    }
    // printf("using font from '%s'\n", path);
    // txfEstablishTexture(txf, 1, GL_TRUE);
    ftxEstablishTexture(ftx, GL_TRUE);
    if (game && game->screen && ftx && ftx->texID) {
      game->screen->texFont = ftx->texID[0];
    }
  } else {
    printf("fatal: could not load font\n");
    exit(1);
  }
}

void deleteFonts() {
  if(ftx != NULL)
    ftxUnloadFont(ftx);
  ftx = NULL;
}
