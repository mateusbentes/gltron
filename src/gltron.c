/*
  gltron
  Copyright (C) 1999 by Andreas Umbach <marvin@dataway.ch>
*/
#include "base/nebu_system.h"
#include "game/init.h"
#include "game/menu.h"
#include "filesystem/path.h"
#include "base/nebu_debug_memory.h"

#ifdef USE_SCRIPTING
#include "scripting/scripting.h"
#endif

int main(int argc, char *argv[] ) {
    nebu_debug_memory_CheckLeaksOnExit();
    
    printf("GLTron starting...\n");

    /* Initialize subsystems */
    initSubsystems(argc, (const char**) argv);
    
    printf("[main] Subsystems initialized successfully\n");
    fflush(stdout);

    /* Initialize menu system */
    printf("[main] Initializing menu system\n");
    initMenu();

    /* Set up menu callbacks */
    printf("[main] Setting up menu callbacks\n");
    fflush(stdout);
    
    printf("[main] Setting display callback\n");
    fflush(stdout);
    nebu_System_SetCallback_Display(drawMenu);
    
    printf("[main] Setting idle callback\n");
    fflush(stdout);
    nebu_System_SetCallback_Idle(menuIdle);
    
    printf("[main] Setting key callback\n");
    fflush(stdout);
    nebu_System_SetCallback_Key(keyMenu);
    
    printf("[main] Setting mouse callback\n");
    fflush(stdout);
    nebu_System_SetCallback_Mouse(mouseMenu);
    
    printf("[main] All callbacks set successfully\n");
    fflush(stdout);

    /* Enter main loop */
    printf("[main] Entering main loop\n");
    fflush(stdout);
    nebu_System_MainLoop();
    printf("[main] Main loop exited\n");
    fflush(stdout);

    /* Exit subsystems */
    exitSubsystems();

    return 0;
}


