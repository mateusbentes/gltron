#include "filesystem/path.h"
#include "filesystem/dirsetup.h"
#include "Nebu_filesystem.h"

#include "stdlib.h"

// Android stub for goto_installpath
void goto_installpath(const char* executable) {
    // On Android, we don't need to change directory
    // Assets are accessed through the Android asset manager
    (void)executable; // Suppress unused parameter warning
}

const char* getHome() {
  return getenv("HOME");
}

void dirSetup(const char *executable) {
#ifdef LOCAL_DATA
	goto_installpath(executable);
#endif
  initDirectories();
}

