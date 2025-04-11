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
    
    /* In a real implementation, you would parse the script and extract configuration values */
    /* For now, just print that we're processing it */
    printf("[embedded] Processed %s script\n", name);
    
    /* Since we're not actually parsing the script, we'll just call scripting_RunString */
    /* This is a temporary solution until proper parsing is implemented */
    scripting_RunString(script);
}

/* Process joystick configuration */
void process_embedded_joystick(void) {
    printf("[embedded] Processing joystick configuration\n");
    const char* script = get_embedded_script("joystick.lua");
    if(script != NULL) {
        printf("[embedded] Found joystick.lua script\n");
        scripting_RunString(script);
    } else {
        fprintf(stderr, "[error] Failed to find embedded script: joystick.lua\n");
    }
}

/* Process path configuration */
void process_embedded_path(void) {
    printf("[embedded] Processing path configuration\n");
    const char* script = get_embedded_script("path.lua");
    if(script) {
        printf("[embedded] Found path.lua script (length: %zu)\n", strlen(script));
        scripting_RunString(script);
    } else {
        fprintf(stderr, "[error] Failed to find embedded script: path.lua\n");
    }
}

/* Process video configuration */
void process_embedded_video(void) {
    printf("[embedded] Processing video configuration\n");
    const char* script = get_embedded_script("video.lua");
    if(script) {
        printf("[embedded] Found video.lua script (length: %zu)\n", strlen(script));
        scripting_RunString(script);
    } else {
        fprintf(stderr, "[error] Failed to find embedded script: video.lua\n");
    }
}

/* Process console configuration */
void process_embedded_console(void) {
    printf("[embedded] Processing console configuration\n");
    const char* script = get_embedded_script("console.lua");
    if(script) {
        printf("[embedded] Found console.lua script (length: %zu)\n", strlen(script));
        scripting_RunString(script);
    } else {
        fprintf(stderr, "[error] Failed to find embedded script: console.lua\n");
    }
}

/* Process menu configuration */
void process_embedded_menu(void) {
    printf("[embedded] Processing menu configuration\n");
    const char* script = get_embedded_script("menu.lua");
    if(script) {
        printf("[embedded] Found menu.lua script (length: %zu)\n", strlen(script));
        scripting_RunString(script);
    } else {
        fprintf(stderr, "[error] Failed to find embedded script: menu.lua\n");
    }
}

/* Process HUD configuration */
void process_embedded_hud(void) {
    printf("[embedded] Processing HUD configuration\n");
    const char* script = get_embedded_script("hud.lua");
    if(script) {
        printf("[embedded] Found hud.lua script (length: %zu)\n", strlen(script));
        scripting_RunString(script);
    } else {
        fprintf(stderr, "[error] Failed to find embedded script: hud.lua\n");
    }
}

/* Process gauge configuration */
void process_embedded_gauge(void) {
    printf("[embedded] Processing gauge configuration\n");
    const char* script = get_embedded_script("gauge.lua");
    if(script) {
        printf("[embedded] Found gauge.lua script (length: %zu)\n", strlen(script));
        scripting_RunString(script);
    } else {
        fprintf(stderr, "[error] Failed to find embedded script: gauge.lua\n");
    }
}

/* Process config configuration */
void process_embedded_config_file(void) {
    printf("[embedded] Processing config configuration\n");
    const char* script = get_embedded_script("config.lua");
    if(script) {
        printf("[embedded] Found config.lua script (length: %zu)\n", strlen(script));
        scripting_RunString(script);
    } else {
        fprintf(stderr, "[error] Failed to find embedded script: config.lua\n");
    }
}

/* Process save configuration */
void process_embedded_save(void) {
    printf("[embedded] Processing save configuration\n");
    const char* script = get_embedded_script("save.lua");
    if(script) {
        printf("[embedded] Found save.lua script (length: %zu)\n", strlen(script));
        scripting_RunString(script);
    } else {
        fprintf(stderr, "[error] Failed to find embedded script: save.lua\n");
    }
}

/* Process artpack configuration */
void process_embedded_artpack(void) {
    printf("[embedded] Processing artpack configuration\n");
    const char* script = get_embedded_script("artpack.lua");
    if(script) {
        printf("[embedded] Found artpack.lua script (length: %zu)\n", strlen(script));
        scripting_RunString(script);
    } else {
        fprintf(stderr, "[error] Failed to find embedded script: artpack.lua\n");
    }
}

/* Process game configuration */
void process_embedded_game(void) {
    printf("[embedded] Processing game configuration\n");
    const char* script = get_embedded_script("game.lua");
    if(script) {
        printf("[embedded] Found game.lua script (length: %zu)\n", strlen(script));
        scripting_RunString(script);
    } else {
        fprintf(stderr, "[error] Failed to find embedded script: game.lua\n");
    }
}

/* Process main configuration */
void process_embedded_main(void) {
    printf("[embedded] Processing main configuration\n");
    const char* script = get_embedded_script("main.lua");
    if(script) {
        printf("[embedded] Found main.lua script (length: %zu)\n", strlen(script));
        scripting_RunString(script);
    } else {
        fprintf(stderr, "[error] Failed to find embedded script: main.lua\n");
    }
}
