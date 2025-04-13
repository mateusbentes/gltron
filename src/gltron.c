/*
  gltron
  Copyright (C) 1999 by Andreas Umbach <marvin@dataway.ch>
*/
#include "base/nebu_system.h" // includes main
#include "game/init.h"
#include "filesystem/path.h"
#include "base/util.h"
#include "base/nebu_debug_memory.h"
#include "scripting/embedded_processing.h" // Added for process_embedded_main()

#include "base/nebu_assert.h"

int main(int argc, char *argv[] ) {
    nebu_debug_memory_CheckLeaksOnExit();
    // nebu_assert_config(NEBU_ASSERT_PRINT_STDERR);
    
    /* Initialize subsystems */
    initSubsystems(argc, (const char**) argv);
    
#ifdef USE_EMBEDDED_SCRIPTS
    /* Process main.lua from embedded scripts */
    printf("[main] Processing embedded main.lua\n");
    process_embedded_main();
#else
    /* Run main.lua from file */
    runScript(PATH_SCRIPTS, "main.lua");
#endif
    
    /* Exit subsystems */
    exitSubsystems();
    
    return 0;
}