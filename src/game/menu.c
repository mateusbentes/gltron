#include <GL/glew.h>  // Include glew.h first
#include <GL/gl.h>    // Then include gl.h
#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "audio/audio.h"
#include "audio/sound_glue.h"
#include "configuration/settings.h"
#include "base/switchCallbacks.h"
#include "game/init.h"
#include "game/gui.h"
#include "game/engine.h"
#include "base/sdl_compat.h"

// Conditional Lua integration - only include if available
#ifdef HAVE_LUA
#include "../lua5/include/lua.h"
#include "../lua5/include/lauxlib.h"
#include "../lua5/include/lualib.h"

// Define LUA_OK if not already defined
#ifndef LUA_OK
#define LUA_OK 0
#endif

// Forward declaration of registerLuaCallbacks
static void registerLuaCallbacks(lua_State *L);
#endif

// Enhanced menu system definitions
typedef enum {
    MENU_TYPE_MENU,
    MENU_TYPE_ACTION,
    MENU_TYPE_LIST,
    MENU_TYPE_SLIDER,
    MENU_TYPE_KEY
} MenuItemType;

typedef enum {
    MENU_STATE_MAIN,
    MENU_STATE_GAME,
    MENU_STATE_VIDEO,
    MENU_STATE_AUDIO,
    MENU_STATE_CONTROLS,
    MENU_STATE_SETTINGS,
    MENU_STATE_HELP,
    MENU_STATE_CREDITS,
    MENU_STATE_RESOLUTION,
    MENU_STATE_DETAILS,
    MENU_STATE_HUD,
    MENU_STATE_GAME_RULES,
    MENU_STATE_GAME_SETTINGS,
    MENU_STATE_KEY_CONFIG,
    MENU_STATE_JOY_CONFIG,
    MENU_STATE_PLAYER_CONFIG,
    MENU_STATE_PLAYER1_KEYS,
    MENU_STATE_PLAYER2_KEYS,
    MENU_STATE_PLAYER3_KEYS,
    MENU_STATE_PLAYER4_KEYS
} MenuState;

// Menu item structure
typedef struct MenuItem {
    char name[64];
    char caption[128];
    MenuItemType type;
    MenuState target_menu;  // For menu type items
    void (*action)(void);   // For action type items
    
    // For list items
    char **labels;
    int *values;
    int num_options;
    int (*read_int)(void);
    void (*store_int)(int value);
    
    // For slider items
    void (*slider_right)(void);
    void (*slider_left)(void);
    char* (*read_string)(void);
    
    // For key items
    int player;
    char event[32];
    
    struct MenuItem *next;
} MenuItem;

// Menu structure
typedef struct Menu {
    char name[64];
    char caption[128];
    MenuState state;
    MenuItem *items;
    int num_items;
    int selected_item;
    struct Menu *parent;
} Menu;

// Global menu variables
static Menu *gCurrentMenu = NULL;
static Menu *gMenus[32];  // Array to hold all menus
static int gNumMenus = 0;
static int gMenuActive = 1;
static TTF_Font *gMenuFont = NULL;
static MenuState gCurrentMenuState = MENU_STATE_MAIN;

#ifdef HAVE_LUA
static lua_State *gLuaState = NULL;
#endif

// Menu navigation history for back functionality
static MenuState gMenuHistory[16];
static int gMenuHistoryIndex = 0;

// Function prototypes
static Menu* createMenu(const char* name, const char* caption, MenuState state);
static MenuItem* createMenuItem(const char* name, const char* caption, MenuItemType type);
static void addMenuItem(Menu* menu, MenuItem* item);
static Menu* findMenu(MenuState state);
static void navigateToMenu(MenuState state);
static void goBack(void);
static void executeMenuItem(MenuItem* item);

// Settings access functions (these would typically be in settings.c)
extern int settings_booster_on;
extern int settings_wall_accel_on;
extern int settings_wall_buster_on;
extern float settings_speed;
extern int settings_ai_level;
extern int settings_erase_crashed;
extern int settings_fast_finish;
extern int settings_camType;
extern int settings_display_type;
extern float settings_map_scale;
extern int settings_players;
extern int settings_ai_opponents;
extern int settings_ai_player1;
extern int settings_ai_player2;
extern int settings_ai_player3;
extern int settings_ai_player4;
extern int settings_mouse_lock_ingame;
extern int settings_mouse_invert_y;
extern int settings_mouse_invert_x;
extern int settings_mipmap_filter;
extern int settings_alpha_trails;
extern int settings_show_glow;
extern int settings_cycle_sharp_edges;
extern float settings_reflection;
extern int settings_show_model;
extern int settings_shadow_volumes_cycle;
extern int settings_arena_outlines;
extern int settings_show_recognizer;
extern int settings_lod;
extern int settings_show_fps;
extern int settings_show_ai_status;
extern int settings_show_scores;
extern int settings_show_speed;
extern int settings_show_console;
extern int settings_show_2d;
extern int settings_fov;
extern int settings_width;
extern int settings_height;
extern int settings_windowMode;
extern int settings_playMusic;
extern int settings_playEffects;
extern float settings_musicVolume;
extern float settings_fxVolume;
extern int settings_loopMusic;
extern char settings_current_level[256];
extern char settings_current_artpack[256];
extern char settings_current_track[256];

// Speed and scale arrays
static float speeds[] = {5.0f, 6.5f, 8.5f, 10.0f};
static float scales[] = {1.0f, 1.5f, 2.0f};

// Callback functions for C integration
void c_quitGame(void) { exit(0); }
void c_startGame(void) { 
    printf("[menu] Starting game...\n");
    gMenuActive = 0;
}
void c_resetGame(void) { printf("[menu] Resetting game...\n"); }
void c_resetScores(void) { printf("[menu] Resetting scores...\n"); }
void c_resetCamera(void) { printf("[menu] Resetting camera...\n"); }
void c_updateUI(void) { printf("[menu] Updating UI...\n"); }
void c_reloadArtpack(void) { printf("[menu] Reloading artpack...\n"); }
void c_video_restart(void) { printf("[menu] Restarting video...\n"); }
void c_timedemo(void) { printf("[menu] Running time demo...\n"); }
void c_update_settings_cache(void) { printf("[menu] Updating settings cache...\n"); }

// Settings getter/setter functions
int get_booster_on(void) { return settings_booster_on; }
void set_booster_on(int value) { settings_booster_on = value; c_resetGame(); }

int get_wall_accel_on(void) { return settings_wall_accel_on; }
void set_wall_accel_on(int value) { settings_wall_accel_on = value; c_resetGame(); }

int get_wall_buster_on(void) { return settings_wall_buster_on; }
void set_wall_buster_on(int value) { settings_wall_buster_on = value; c_resetGame(); }

int get_speed_index(void) {
    if (settings_speed <= 5.0) return 0;      // boring
    else if (settings_speed <= 6.5) return 1; // normal
    else if (settings_speed <= 8.5) return 2; // fast
    else return 3;                             // crazy
}

void set_speed_index(int value) {
    if (value >= 0 && value < 4) {
        settings_speed = speeds[value];
        c_resetGame();
    }
}

int get_ai_level(void) { return settings_ai_level; }
void set_ai_level(int value) { settings_ai_level = value; }

int get_erase_crashed(void) { return settings_erase_crashed; }
void set_erase_crashed(int value) { settings_erase_crashed = value; c_resetGame(); }

int get_fast_finish(void) { return settings_fast_finish; }
void set_fast_finish(int value) { settings_fast_finish = value; }

int get_camType(void) { return settings_camType; }
void set_camType(int value) { settings_camType = value; c_resetCamera(); }

int get_display_type(void) { return settings_display_type; }
void set_display_type(int value) { settings_display_type = value; c_updateUI(); }

int get_map_scale_index(void) {
    if (settings_map_scale <= 1.0) return 0;      // small
    else if (settings_map_scale <= 1.5) return 1; // medium
    else return 2;                                 // huge
}

void set_map_scale_index(int value) {
    if (value >= 0 && value < 3) {
        settings_map_scale = scales[value];
    }
}

// Audio settings
int get_playMusic(void) { return settings_playMusic; }
void set_playMusic(int value) { settings_playMusic = value; c_update_settings_cache(); }

int get_playEffects(void) { return settings_playEffects; }
void set_playEffects(int value) { settings_playEffects = value; }

int get_loopMusic(void) { return settings_loopMusic; }
void set_loopMusic(int value) { settings_loopMusic = value; }

// Video settings
int get_mipmap_filter(void) { return settings_mipmap_filter; }
void set_mipmap_filter(int value) { settings_mipmap_filter = value; c_reloadArtpack(); }

int get_alpha_trails(void) { return settings_alpha_trails; }
void set_alpha_trails(int value) { settings_alpha_trails = value; }

int get_show_glow(void) { return settings_show_glow; }
void set_show_glow(int value) { settings_show_glow = value; }

// Slider functions
void MusicVolumeUp(void) {
    if (settings_musicVolume < 1.0) settings_musicVolume += 0.1;
}
void MusicVolumeDown(void) {
    if (settings_musicVolume > 0.0) settings_musicVolume -= 0.1;
}

void FXVolumeUp(void) {
    if (settings_fxVolume < 1.0) settings_fxVolume += 0.1;
}
void FXVolumeDown(void) {
    if (settings_fxVolume > 0.0) settings_fxVolume -= 0.1;
}

char* get_music_volume_string(void) {
    static char buffer[32];
    snprintf(buffer, sizeof(buffer), "%.0f%%", settings_musicVolume * 100);
    return buffer;
}

char* get_fx_volume_string(void) {
    static char buffer[32];
    snprintf(buffer, sizeof(buffer), "%.0f%%", settings_fxVolume * 100);
    return buffer;
}

void nextLevel(void) {
    printf("[menu] Next level\n");
    // Implementation would cycle through available levels
}

void previousLevel(void) {
    printf("[menu] Previous level\n");
    // Implementation would cycle through available levels
}

char* get_current_level(void) {
    static char buffer[128];
    // Extract name without extension
    char *dot = strrchr(settings_current_level, '.');
    if (dot) {
        int len = dot - settings_current_level;
        strncpy(buffer, settings_current_level, len);
        buffer[len] = '\0';
    } else {
        strcpy(buffer, settings_current_level);
    }
    return buffer;
}

void nextArtpack(void) {
    printf("[menu] Next artpack\n");
    // Implementation would cycle through available artpacks
}

void previousArtpack(void) {
    printf("[menu] Previous artpack\n");
    // Implementation would cycle through available artpacks
}

char* get_current_artpack(void) {
    return settings_current_artpack;
}

void nextTrack(void) {
    printf("[menu] Next track\n");
    // Implementation would cycle through available tracks
}

void previousTrack(void) {
    printf("[menu] Previous track\n");
    // Implementation would cycle through available tracks
}

char* get_current_track(void) {
    static char buffer[128];
    // Extract name without extension
    char *dot = strrchr(settings_current_track, '.');
    if (dot) {
        int len = dot - settings_current_track;
        strncpy(buffer, settings_current_track, len);
        buffer[len] = '\0';
    } else {
        strcpy(buffer, settings_current_track);
    }
    return buffer;
}

// Basic input function implementation
int nebu_Input_KeyPressed(int key) {
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    return state[SDL_GetScancodeFromKey(key)];
}

#ifdef HAVE_LUA
// Enhanced Lua integration
static void loadLuaMenuDefinitions(void) {
    if (!gLuaState) return;
    
    // Load music functions first
    if (luaL_dofile(gLuaState, "scripts/music_functions.lua") != LUA_OK) {
        printf("[menu] Warning: Could not load music_functions.lua: %s\n", 
               lua_tostring(gLuaState, -1));
        lua_pop(gLuaState, 1);
    }
    
    // Execute the main menu script
    if (luaL_dofile(gLuaState, "scripts/menu.lua") != LUA_OK) {
        printf("[menu] Error loading menu.lua: %s\n", lua_tostring(gLuaState, -1));
        lua_pop(gLuaState, 1);
        return;
    }

    // Get the MainGameMenu table
    lua_getglobal(gLuaState, "MainGameMenu");
    if (!lua_istable(gLuaState, -1)) {
        printf("[menu] MainGameMenu not found in menu.lua\n");
        lua_pop(gLuaState, 1);
        return;
    }

    // Set current menu
    lua_getfield(gLuaState, -1, "current");
    if (lua_isstring(gLuaState, -1)) {
        const char *current = lua_tostring(gLuaState, -1);
        printf("[menu] Current menu: %s\n", current);
    }
    lua_pop(gLuaState, 1);

    // Process menu definitions
    lua_pushnil(gLuaState);
    while (lua_next(gLuaState, -2) != 0) {
        const char *key = lua_tostring(gLuaState, -2);

        if (lua_istable(gLuaState, -1)) {
            // Check if this is a menu definition
            lua_getfield(gLuaState, -1, "type");
            if (lua_isnumber(gLuaState, -1)) {
                int type = lua_tointeger(gLuaState, -1);
                printf("[menu] Found item '%s' of type %d\n", key, type);
                
                // Get caption
                lua_pop(gLuaState, 1);
                lua_getfield(gLuaState, -1, "caption");
                const char *caption = lua_isstring(gLuaState, -1) ? lua_tostring(gLuaState, -1) : key;
                printf("[menu] Caption: %s\n", caption);
                lua_pop(gLuaState, 1);
                
                // Get items if it's a menu
                if (type == 0) { // MenuC.type.menu
                    lua_getfield(gLuaState, -1, "items");
                    if (lua_istable(gLuaState, -1)) {
                        int num_items = lua_objlen(gLuaState, -1);
                        printf("[menu] Menu %s has %d items\n", key, num_items);
                        
                        // Process each menu item
                        for (int i = 1; i <= num_items; i++) {
                            lua_rawgeti(gLuaState, -1, i);
                            const char *item_name = lua_tostring(gLuaState, -1);
                            printf("[menu] Menu item: %s\n", item_name);
                            lua_pop(gLuaState, 1);
                        }
                    }
                    lua_pop(gLuaState, 1);
                }
            } else {
                lua_pop(gLuaState, 1);
            }
        }

        lua_pop(gLuaState, 1);
    }
    
    lua_pop(gLuaState, 1); // Pop MainGameMenu table
}

// Enhanced Lua callback functions
static int lua_c_quitGame(lua_State *L) {
    c_quitGame();
    return 0;
}

static int lua_c_startGame(lua_State *L) {
    c_startGame();
    return 0;
}

static int lua_c_resetScores(lua_State *L) {
    c_resetScores();
    return 0;
}

static int lua_c_resetGame(lua_State *L) {
    c_resetGame();
    return 0;
}

static int lua_c_resetCamera(lua_State *L) {
    c_resetCamera();
    return 0;
}

static int lua_c_updateUI(lua_State *L) {
    c_updateUI();
    return 0;
}

static int lua_c_reloadArtpack(lua_State *L) {
    c_reloadArtpack();
    return 0;
}

static int lua_c_video_restart(lua_State *L) {
    c_video_restart();
    return 0;
}

static int lua_c_timedemo(lua_State *L) {
    c_timedemo();
    return 0;
}

static int lua_c_update_settings_cache(lua_State *L) {
    c_update_settings_cache();
    return 0;
}

// Register enhanced Lua callback functions
static void registerLuaCallbacks(lua_State *L) {
    lua_register(L, "c_quitGame", lua_c_quitGame);
    lua_register(L, "c_startGame", lua_c_startGame);
    lua_register(L, "c_resetScores", lua_c_resetScores);
    lua_register(L, "c_resetGame", lua_c_resetGame);
    lua_register(L, "c_resetCamera", lua_c_resetCamera);
    lua_register(L, "c_updateUI", lua_c_updateUI);
    lua_register(L, "c_reloadArtpack", lua_c_reloadArtpack);
    lua_register(L, "c_video_restart", lua_c_video_restart);
    lua_register(L, "c_timedemo", lua_c_timedemo);
    lua_register(L, "c_update_settings_cache", lua_c_update_settings_cache);
    
    // Create MenuC table with type constants
    lua_newtable(L);
    lua_newtable(L);
    lua_pushnumber(L, MENU_TYPE_MENU); lua_setfield(L, -2, "menu");
    lua_pushnumber(L, MENU_TYPE_ACTION); lua_setfield(L, -2, "action");
    lua_pushnumber(L, MENU_TYPE_LIST); lua_setfield(L, -2, "list");
    lua_pushnumber(L, MENU_TYPE_SLIDER); lua_setfield(L, -2, "slider");
    lua_pushnumber(L, MENU_TYPE_KEY); lua_setfield(L, -2, "key");
    lua_setfield(L, -2, "type");
    lua_setglobal(L, "MenuC");
}
#endif

// Menu creation and management functions
static Menu* createMenu(const char* name, const char* caption, MenuState state) {
    Menu* menu = malloc(sizeof(Menu));
    if (!menu) return NULL;
    
    strncpy(menu->name, name, sizeof(menu->name) - 1);
    menu->name[sizeof(menu->name) - 1] = '\0';
    strncpy(menu->caption, caption, sizeof(menu->caption) - 1);
    menu->caption[sizeof(menu->caption) - 1] = '\0';
    menu->state = state;
    menu->items = NULL;
    menu->num_items = 0;
    menu->selected_item = 0;
    menu->parent = NULL;
    
    return menu;
}

static MenuItem* createMenuItem(const char* name, const char* caption, MenuItemType type) {
    MenuItem* item = malloc(sizeof(MenuItem));
    if (!item) return NULL;
    
    memset(item, 0, sizeof(MenuItem));
    strncpy(item->name, name, sizeof(item->name) - 1);
    item->name[sizeof(item->name) - 1] = '\0';
    strncpy(item->caption, caption, sizeof(item->caption) - 1);
    item->caption[sizeof(item->caption) - 1] = '\0';
    item->type = type;
    item->next = NULL;
    
    return item;
}

static void addMenuItem(Menu* menu, MenuItem* item) {
    if (!menu || !item) return;
    
    if (!menu->items) {
        menu->items = item;
    } else {
        MenuItem* current = menu->items;
        while (current->next) {
            current = current->next;
        }
        current->next = item;
    }
    menu->num_items++;
}

static Menu* findMenu(MenuState state) {
    for (int i = 0; i < gNumMenus; i++) {
        if (gMenus[i] && gMenus[i]->state == state) {
            return gMenus[i];
        }
    }
    return NULL;
}

static void navigateToMenu(MenuState state) {
    Menu* menu = findMenu(state);
    if (menu) {
        // Save current menu to history
        if (gMenuHistoryIndex < 15) {
            gMenuHistory[gMenuHistoryIndex++] = gCurrentMenuState;
        }
        
        gCurrentMenu = menu;
        gCurrentMenuState = state;
        gCurrentMenu->selected_item = 0;
    }
}

static void goBack(void) {
    if (gMenuHistoryIndex > 0) {
        MenuState prevState = gMenuHistory[--gMenuHistoryIndex];
        Menu* menu = findMenu(prevState);
        if (menu) {
            gCurrentMenu = menu;
            gCurrentMenuState = prevState;
        }
    }
}

static void executeMenuItem(MenuItem* item) {
    if (!item) return;
    
    switch (item->type) {
        case MENU_TYPE_MENU:
            navigateToMenu(item->target_menu);
            break;
        case MENU_TYPE_ACTION:
            if (item->action) {
                item->action();
            }
            break;
        case MENU_TYPE_LIST:
            // Toggle through list options
            if (item->read_int && item->store_int && item->values && item->num_options > 0) {
                int current = item->read_int();
                int next_index = 0;
                
                // Find current index
                for (int i = 0; i < item->num_options; i++) {
                    if (item->values[i] == current) {
                        next_index = (i + 1) % item->num_options;
                        break;
                    }
                }
                
                item->store_int(item->values[next_index]);
            }
            break;
        case MENU_TYPE_SLIDER:
            if (item->slider_right) {
                item->slider_right();
            }
            break;
        case MENU_TYPE_KEY:
            // Handle key configuration
            printf("[menu] Configuring key for player %d, event %s\n", item->player, item->event);
            break;
    }
}

// Initialize comprehensive menu system
static void initMenuSystem(void) {
    // Create all menus
    gMenus[gNumMenus++] = createMenu("RootMenu", "", MENU_STATE_MAIN);
    gMenus[gNumMenus++] = createMenu("GameMenu", "Game", MENU_STATE_GAME);
    gMenus[gNumMenus++] = createMenu("VideoMenu", "Video", MENU_STATE_VIDEO);
    gMenus[gNumMenus++] = createMenu("AudioMenu", "Audio", MENU_STATE_AUDIO);
    gMenus[gNumMenus++] = createMenu("ControlsMenu", "Controls", MENU_STATE_CONTROLS);
    gMenus[gNumMenus++] = createMenu("GameRulesMenu", "Game Rules", MENU_STATE_GAME_RULES);
    gMenus[gNumMenus++] = createMenu("GameSettingsMenu", "Play Settings", MENU_STATE_GAME_SETTINGS);
    gMenus[gNumMenus++] = createMenu("ResolutionMenu", "Screen", MENU_STATE_RESOLUTION);
    gMenus[gNumMenus++] = createMenu("DetailsMenu", "Details", MENU_STATE_DETAILS);
    gMenus[gNumMenus++] = createMenu("HudMenu", "HUD", MENU_STATE_HUD);
    
    // Set current menu
    gCurrentMenu = findMenu(MENU_STATE_MAIN);
    gCurrentMenuState = MENU_STATE_MAIN;
    
    // Create main menu items
    Menu* mainMenu = findMenu(MENU_STATE_MAIN);
    if (mainMenu) {
        MenuItem* gameItem = createMenuItem("GameMenu", "Game", MENU_TYPE_MENU);
        gameItem->target_menu = MENU_STATE_GAME;
        addMenuItem(mainMenu, gameItem);
        
        MenuItem* videoItem = createMenuItem("VideoMenu", "Video", MENU_TYPE_MENU);
        videoItem->target_menu = MENU_STATE_VIDEO;
        addMenuItem(mainMenu, videoItem);
        
        MenuItem* audioItem = createMenuItem("AudioMenu", "Audio", MENU_TYPE_MENU);
        audioItem->target_menu = MENU_STATE_AUDIO;
        addMenuItem(mainMenu, audioItem);
        
        MenuItem* controlsItem = createMenuItem("ControlsMenu", "Controls", MENU_TYPE_MENU);
        controlsItem->target_menu = MENU_STATE_CONTROLS;
        addMenuItem(mainMenu, controlsItem);
        
        MenuItem* quitItem = createMenuItem("Quit", "Quit", MENU_TYPE_ACTION);
        quitItem->action = c_quitGame;
        addMenuItem(mainMenu, quitItem);
    }
    
    // Create game menu items
    Menu* gameMenu = findMenu(MENU_STATE_GAME);
    if (gameMenu) {
        MenuItem* startItem = createMenuItem("StartGame", "Start Game", MENU_TYPE_ACTION);
        startItem->action = c_startGame;
        addMenuItem(gameMenu, startItem);
        
        MenuItem* rulesItem = createMenuItem("GameRulesMenu", "Game Rules", MENU_TYPE_MENU);
        rulesItem->target_menu = MENU_STATE_GAME_RULES;
        addMenuItem(gameMenu, rulesItem);
        
        MenuItem* settingsItem = createMenuItem("GameSettingsMenu", "Play Settings", MENU_TYPE_MENU);
        settingsItem->target_menu = MENU_STATE_GAME_SETTINGS;
        addMenuItem(gameMenu, settingsItem);
        
        MenuItem* resetItem = createMenuItem("ResetScores", "Reset Scores", MENU_TYPE_ACTION);
        resetItem->action = c_resetScores;
        addMenuItem(gameMenu, resetItem);
    }
    
    // Create game rules menu items
    Menu* rulesMenu = findMenu(MENU_STATE_GAME_RULES);
    if (rulesMenu) {
        MenuItem* boosterItem = createMenuItem("Booster", "Booster", MENU_TYPE_LIST);
        boosterItem->labels = malloc(2 * sizeof(char*));
        boosterItem->labels[0] = "off";
        boosterItem->labels[1] = "on";
        boosterItem->values = malloc(2 * sizeof(int));
        boosterItem->values[0] = 0;
        boosterItem->values[1] = 1;
        boosterItem->num_options = 2;
        boosterItem->read_int = get_booster_on;
        boosterItem->store_int = set_booster_on;
        addMenuItem(rulesMenu, boosterItem);
        
        MenuItem* speedItem = createMenuItem("GameSpeed", "Game speed", MENU_TYPE_LIST);
        speedItem->labels = malloc(4 * sizeof(char*));
        speedItem->labels[0] = "boring";
        speedItem->labels[1] = "normal";
        speedItem->labels[2] = "fast";
        speedItem->labels[3] = "crazy";
        speedItem->values = malloc(4 * sizeof(int));
        speedItem->values[0] = 0;
        speedItem->values[1] = 1;
        speedItem->values[2] = 2;
        speedItem->values[3] = 3;
        speedItem->num_options = 4;
        speedItem->read_int = get_speed_index;
        speedItem->store_int = set_speed_index;
        addMenuItem(rulesMenu, speedItem);
    }
    
    // Create audio menu items
    Menu* audioMenu = findMenu(MENU_STATE_AUDIO);
    if (audioMenu) {
        MenuItem* musicItem = createMenuItem("Music", "Music", MENU_TYPE_LIST);
        musicItem->labels = malloc(2 * sizeof(char*));
        musicItem->labels[0] = "off";
        musicItem->labels[1] = "on";
        musicItem->values = malloc(2 * sizeof(int));
        musicItem->values[0] = 0;
        musicItem->values[1] = 1;
        musicItem->num_options = 2;
        musicItem->read_int = get_playMusic;
        musicItem->store_int = set_playMusic;
        addMenuItem(audioMenu, musicItem);
        
        MenuItem* volumeItem = createMenuItem("Music_Volume", "Music Volume", MENU_TYPE_SLIDER);
        volumeItem->slider_right = MusicVolumeUp;
        volumeItem->slider_left = MusicVolumeDown;
        volumeItem->read_string = get_music_volume_string;
        addMenuItem(audioMenu, volumeItem);
        
        MenuItem* songItem = createMenuItem("Song", "Song", MENU_TYPE_SLIDER);
        songItem->slider_right = nextTrack;
        songItem->slider_left = previousTrack;
        songItem->read_string = get_current_track;
        addMenuItem(audioMenu, songItem);
    }
}

// Font initialization
void initMenuFont(void) {
    if (TTF_Init() == -1) {
        printf("[menu] Failed to initialize SDL_ttf: %s\n", TTF_GetError());
        return;
    }

    gMenuFont = TTF_OpenFont("data/fonts/DejaVuSans.ttf", 24);
    if (!gMenuFont) {
        printf("[menu] Failed to load font: %s\n", TTF_GetError());
        gMenuFont = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 24);
        if (!gMenuFont) {
            printf("[menu] Failed to load default font: %s\n", TTF_GetError());
        }
    }
}

// Text rendering
void renderText(const char *text, int x, int y, SDL_Color color) {
    if (!gMenuFont || !text) {
        return;
    }

    SDL_Surface *textSurface = TTF_RenderText_Solid(gMenuFont, text, color);
    if (!textSurface) {
        return;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textSurface->w, textSurface->h, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, textSurface->pixels);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex2f(x, y);
    glTexCoord2f(1, 0); glVertex2f(x + textSurface->w, y);
    glTexCoord2f(1, 1); glVertex2f(x + textSurface->w, y + textSurface->h);
    glTexCoord2f(0, 1); glVertex2f(x, y + textSurface->h);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);

    glDeleteTextures(1, &texture);
    SDL_FreeSurface(textSurface);
}

// Main initialization function
void initMenu(void) {
    printf("[menu] Initializing enhanced menu system\n");
    
    // Initialize basic menu system
    initMenuSystem();
    
    // Initialize font
    initMenuFont();

#ifdef HAVE_LUA
    // Initialize Lua state
    gLuaState = luaL_newstate();
    if (gLuaState) {
        luaL_openlibs(gLuaState);
        registerLuaCallbacks(gLuaState);
        loadLuaMenuDefinitions();
        printf("[menu] Lua integration initialized\n");
    } else {
        printf("[menu] Failed to create Lua state\n");
    }
#else
    printf("[menu] Lua integration disabled (not compiled with Lua support)\n");
#endif

    printf("[menu] Enhanced menu system initialized successfully\n");
}

// Enhanced menu drawing
void drawMenu(void) {
    if (!gCurrentMenu) return;
    
    int screenWidth, screenHeight;
    nebu_Video_GetDimension(&screenWidth, &screenHeight);
    
    if (screenWidth <= 0 || screenHeight <= 0) {
        screenWidth = 1024;
        screenHeight = 768;
    }

    glViewport(0, 0, screenWidth, screenHeight);
    glClearColor(0.0f, 0.0f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, screenWidth, screenHeight, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Draw background gradient
    glBegin(GL_QUADS);
    glColor4f(0.0f, 0.0f, 0.2f, 1.0f);
    glVertex2f(0, 0);
    glVertex2f(screenWidth, 0);
    glColor4f(0.0f, 0.0f, 0.4f, 1.0f);
    glVertex2f(screenWidth, screenHeight);
    glVertex2f(0, screenHeight);
    glEnd();

    // Draw menu title
    if (strlen(gCurrentMenu->caption) > 0) {
        SDL_Color titleColor = {255, 255, 255, 255};
        renderText(gCurrentMenu->caption, screenWidth/2 - 100, 50, titleColor);
    }

    // Draw menu items
    MenuItem* item = gCurrentMenu->items;
    int itemIndex = 0;
    int startY = 150;
    
    while (item) {
        int y = startY + itemIndex * 50;
        
        // Highlight selected item
        if (itemIndex == gCurrentMenu->selected_item) {
            glColor4f(0.0f, 0.5f, 1.0f, 0.7f);
            glBegin(GL_QUADS);
            glVertex2f(screenWidth/2 - 200, y - 5);
            glVertex2f(screenWidth/2 + 200, y - 5);
            glVertex2f(screenWidth/2 + 200, y + 35);
            glVertex2f(screenWidth/2 - 200, y + 35);
            glEnd();
            
            // Draw selection arrow
            glColor4f(1.0f, 1.0f, 0.0f, 1.0f);
            glBegin(GL_TRIANGLES);
            glVertex2f(screenWidth/2 - 220, y + 15);
            glVertex2f(screenWidth/2 - 200, y + 25);
            glVertex2f(screenWidth/2 - 200, y + 5);
            glEnd();
        }
        
        // Draw item text
        SDL_Color textColor = (itemIndex == gCurrentMenu->selected_item) ? 
                             (SDL_Color){255, 255, 255, 255} : 
                             (SDL_Color){180, 180, 180, 255};
        
        char displayText[256];
        strcpy(displayText, item->caption);
        
        // Add value display for list and slider items
        if (item->type == MENU_TYPE_LIST && item->read_int && item->labels) {
            int currentValue = item->read_int();
            for (int i = 0; i < item->num_options; i++) {
                if (item->values[i] == currentValue) {
                    snprintf(displayText, sizeof(displayText), "%s: %s", 
                            item->caption, item->labels[i]);
                    break;
                }
            }
        } else if (item->type == MENU_TYPE_SLIDER && item->read_string) {
            char* value = item->read_string();
            snprintf(displayText, sizeof(displayText), "%s: %s", 
                    item->caption, value);
        }
        
        renderText(displayText, screenWidth/2 - 150, y + 5, textColor);
        
        item = item->next;
        itemIndex++;
    }

    glEnable(GL_DEPTH_TEST);
    nebu_System_SwapBuffers();
}

// Navigation functions
void navigateMenu(int direction) {
    if (!gCurrentMenu) return;
    
    gCurrentMenu->selected_item += direction;
    
    if (gCurrentMenu->selected_item < 0) {
        gCurrentMenu->selected_item = gCurrentMenu->num_items - 1;
    } else if (gCurrentMenu->selected_item >= gCurrentMenu->num_items) {
        gCurrentMenu->selected_item = 0;
    }
}

void selectMenuOption(void) {
    if (!gCurrentMenu) return;
    
    MenuItem* item = gCurrentMenu->items;
    for (int i = 0; i < gCurrentMenu->selected_item && item; i++) {
        item = item->next;
    }
    
    if (item) {
        executeMenuItem(item);
    }
}

// Input handling
void handleMenuInput(void) {
    if (nebu_Input_KeyPressed(SDLK_UP)) {
        navigateMenu(-1);
    } else if (nebu_Input_KeyPressed(SDLK_DOWN)) {
        navigateMenu(1);
    } else if (nebu_Input_KeyPressed(SDLK_LEFT)) {
        // Handle left navigation for sliders and lists
        if (gCurrentMenu) {
            MenuItem* item = gCurrentMenu->items;
            for (int i = 0; i < gCurrentMenu->selected_item && item; i++) {
                item = item->next;
            }
            if (item && item->type == MENU_TYPE_SLIDER && item->slider_left) {
                item->slider_left();
            } else if (item && item->type == MENU_TYPE_LIST) {
                // Navigate backwards through list
                if (item->read_int && item->store_int && item->values && item->num_options > 0) {
                    int current = item->read_int();
                    int prev_index = item->num_options - 1;
                    
                    for (int i = 0; i < item->num_options; i++) {
                        if (item->values[i] == current) {
                            prev_index = (i - 1 + item->num_options) % item->num_options;
                            break;
                        }
                    }
                    
                    item->store_int(item->values[prev_index]);
                }
            }
        }
    } else if (nebu_Input_KeyPressed(SDLK_RIGHT)) {
        // Handle right navigation for sliders and lists
        if (gCurrentMenu) {
            MenuItem* item = gCurrentMenu->items;
            for (int i = 0; i < gCurrentMenu->selected_item && item; i++) {
                item = item->next;
            }
            if (item && item->type == MENU_TYPE_SLIDER && item->slider_right) {
                item->slider_right();
            } else if (item && item->type == MENU_TYPE_LIST) {
                executeMenuItem(item); // Same as enter for lists
            }
        }
    } else if (nebu_Input_KeyPressed(SDLK_RETURN)) {
        selectMenuOption();
    } else if (nebu_Input_KeyPressed(SDLK_ESCAPE)) {
        goBack();
    }
}

// Main menu functions
void menuMain(void) {
    if (gMenuActive) {
        handleMenuInput();
        drawMenu();
    }
}

void menuIdle(void) {
    if (gMenuActive) {
        menuMain();
    }
}

void keyMenu(int state, int key, int x, int y) {
    if (state == SDL_PRESSED) {
        switch (key) {
            case SDLK_UP:
                navigateMenu(-1);
                break;
            case SDLK_DOWN:
                navigateMenu(1);
                break;
            case SDLK_LEFT:
            case SDLK_RIGHT:
            case SDLK_RETURN:
                // These are handled in handleMenuInput
                break;
            case SDLK_ESCAPE:
                goBack();
                break;
        }
    }
}

void mouseMenu(int button, int state, int x, int y) {
    if (button == SDL_BUTTON_LEFT && state == SDL_PRESSED && gCurrentMenu) {
        int screenWidth, screenHeight;
        nebu_Video_GetDimension(&screenWidth, &screenHeight);
        
        int startY = 150;
        for (int i = 0; i < gCurrentMenu->num_items; i++) {
            int itemY = startY + i * 50;
            if (y >= itemY - 5 && y <= itemY + 35 &&
                x >= screenWidth/2 - 200 && x <= screenWidth/2 + 200) {
                gCurrentMenu->selected_item = i;
                selectMenuOption();
                break;
            }
        }
    }
}

// Utility functions
int isMenuActive(void) {
    return gMenuActive;
}

void returnToMenu(void) {
    gCurrentMenuState = MENU_STATE_MAIN;
    gCurrentMenu = findMenu(MENU_STATE_MAIN);
    gMenuActive = 1;
}

void exitGame(void) {
    exit(0);
}

// Cleanup function
void cleanupMenu(void) {
    if (gMenuFont) {
        TTF_CloseFont(gMenuFont);
        gMenuFont = NULL;
    }
    
    TTF_Quit();

#ifdef HAVE_LUA
    if (gLuaState) {
        lua_close(gLuaState);
        gLuaState = NULL;
    }
#endif

    // Free menu structures
    for(int i = 0; i < gNumMenus; i++) {
        if (gMenus[i]) {
            MenuItem* item = gMenus[i]->items;
            while (item) {
                MenuItem* next = item->next;
                if (item->labels) free(item->labels);
                if (item->values) free(item->values);
                free(item);
                item = next;
            }
            free(gMenus[i]);
            gMenus[i] = NULL;
        }
    }
}
