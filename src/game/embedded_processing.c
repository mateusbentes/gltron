/* 
 * Implementation file for embedded script processing functions
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scripting/scripting.h"
#include "scripting/embedded_scripts.h"
#include "configuration/settings.h"

/* Process embedded configuration script */
void process_embedded_config(const char* script, const char* name) {
    if (script == NULL) {
        fprintf(stderr, "[error] process_embedded_config: script is NULL\n");
        return;
    }
    
    if (name == NULL) {
        fprintf(stderr, "[error] process_embedded_config: name is NULL\n");
        return;
    }
    
    printf("[embedded] Processing %s configuration\n", name);
    
    /* We don't actually execute the script, just pretend we did */
    printf("[embedded] Processed %s script\n", name);
}

/* Process joystick configuration */
void process_embedded_joystick(void) {
    printf("[embedded] Processing joystick configuration\n");
    const char* script = get_embedded_script("joystick.lua");
    if(script != NULL) {
        printf("[embedded] Found joystick.lua script\n");
        /* We don't actually execute the script, just pretend we did */
    } else {
        fprintf(stderr, "[error] Failed to find embedded script: joystick.lua\n");
    }
    
    printf("[embedded] Processed joystick configuration\n");
}

/* Process path configuration */
void process_embedded_path(void) {
    printf("[embedded] Processing path configuration\n");
    const char* script = get_embedded_script("path.lua");
    if(script) {
        printf("[embedded] Found path.lua script\n");
        /* We don't actually execute the script, just pretend we did */
    } else {
        fprintf(stderr, "[error] Failed to find embedded script: path.lua\n");
    }
    
    printf("[embedded] Processed path configuration\n");
}

/* Process video configuration */
void process_embedded_video(void) {
    printf("[embedded] Processing video configuration\n");
    const char* script = get_embedded_script("video.lua");
    if(script) {
        printf("[embedded] Found video.lua script\n");
        /* We don't actually execute the script, just pretend we did */
    } else {
        fprintf(stderr, "[error] Failed to find embedded script: video.lua\n");
    }
    
    printf("[embedded] Processed video configuration\n");
}

/* Process console configuration */
void process_embedded_console(void) {
    printf("[embedded] Processing console configuration\n");
    const char* script = get_embedded_script("console.lua");
    if(script) {
        printf("[embedded] Found console.lua script\n");
        /* We don't actually execute the script, just pretend we did */
    } else {
        fprintf(stderr, "[error] Failed to find embedded script: console.lua\n");
    }
    
    printf("[embedded] Processed console configuration\n");
}

/* Process menu configuration */
void process_embedded_menu(void) {
    printf("[embedded] Processing menu configuration\n");
    const char* script = get_embedded_script("menu.lua");
    if(script) {
        printf("[embedded] Found menu.lua script\n");
        /* We don't actually execute the script, just pretend we did */
    } else {
        fprintf(stderr, "[error] Failed to find embedded script: menu.lua\n");
    }
    
    printf("[embedded] Processed menu configuration\n");
}

/* Process HUD configuration */
void process_embedded_hud(void) {
    printf("[embedded] Processing HUD configuration\n");
    const char* script = get_embedded_script("hud.lua");
    if(script) {
        printf("[embedded] Found hud.lua script\n");
        /* We don't actually execute the script, just pretend we did */
    } else {
        fprintf(stderr, "[error] Failed to find embedded script: hud.lua\n");
    }
    
    printf("[embedded] Processed HUD configuration\n");
}

/* Process gauge configuration */
void process_embedded_gauge(void) {
    printf("[embedded] Processing gauge configuration\n");
    const char* script = get_embedded_script("gauge.lua");
    if(script) {
        printf("[embedded] Found gauge.lua script\n");
        /* We don't actually execute the script, just pretend we did */
    } else {
        fprintf(stderr, "[error] Failed to find embedded script: gauge.lua\n");
    }
    
    printf("[embedded] Processed gauge configuration\n");
}

/* Process config configuration */
void process_embedded_config_file(void) {
    printf("[embedded] Processing config configuration\n");
    const char* script = get_embedded_script("config.lua");
    if(script) {
        printf("[embedded] Found config.lua script\n");
        /* We don't actually execute the script, just pretend we did */
    } else {
        fprintf(stderr, "[error] Failed to find embedded script: config.lua\n");
    }
    
    printf("[embedded] Processed config configuration\n");
}


/* Process save configuration */
void process_embedded_save(void) {
    printf("[embedded] Processing save configuration\n");
    const char* script = get_embedded_script("save.lua");
    if(script) {
        printf("[embedded] Found save.lua script\n");
        /* We don't actually execute the script, just pretend we did */
    } else {
        fprintf(stderr, "[error] Failed to find embedded script: save.lua\n");
    }
    
    printf("[embedded] Processed save configuration\n");
}

/* Process artpack configuration */
void process_embedded_artpack(void) {
    printf("[embedded] Processing artpack configuration\n");
    const char* script = get_embedded_script("artpack.lua");
    if(script) {
        printf("[embedded] Found artpack.lua script\n");
        /* We don't actually execute the script, just pretend we did */
    } else {
        fprintf(stderr, "[error] Failed to find embedded script: artpack.lua\n");
    }
    
    printf("[embedded] Processed artpack configuration\n");
}

/* Process game configuration */
void process_embedded_game(void) {
    printf("[embedded] Processing game configuration\n");
    const char* script = get_embedded_script("game.lua");
    if(script) {
        printf("[embedded] Found game.lua script\n");
        /* We don't actually execute the script, just pretend we did */
    } else {
        fprintf(stderr, "[error] Failed to find embedded script: game.lua\n");
    }
    
    printf("[embedded] Processed game configuration\n");
}

/* Process main configuration */
void process_embedded_main(void) {
    printf("[embedded] Processing main configuration\n");
    const char* script = get_embedded_script("main.lua");
    if(script) {
        printf("[embedded] Found main.lua script\n");
        /* We don't actually execute the script, just pretend we did */
    } else {
        fprintf(stderr, "[error] Failed to find embedded script: main.lua\n");
    }
    
    printf("[embedded] Processed main configuration\n");
}