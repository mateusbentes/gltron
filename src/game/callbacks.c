#include "game/init.h"
#include <stdio.h>

int runGame(void) {
    printf("[game] Running game callback\n");
    return 1;  /* Continue running */
}

int runGUI(void) {
    printf("[game] Running GUI callback\n");
    return 1;  /* Continue running */
}

int runPause(void) {
    printf("[game] Running pause callback\n");
    return 1;  /* Continue running */
}

int runConfigure(void) {
    printf("[game] Running configure callback\n");
    return 1;  /* Continue running */
}

int runCredits(void) {
    printf("[game] Running credits callback\n");
    return 1;  /* Continue running */
}

int runTimedemo(void) {
    printf("[game] Running timedemo callback\n");
    return 0;  /* Quit after timedemo */
}