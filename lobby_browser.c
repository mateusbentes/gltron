#include "gltron.h"
#include <stdio.h>
#include <string.h>

#ifdef USE_STEAMWORKS

/* Maximum number of lobbies to display */
#define MAX_LOBBY_LIST 10

typedef struct {
    uint64_t lobby_id;
    char name[64];
    int current_players;
    int max_players;
    char host_name[64];
} LobbyEntry;

static LobbyEntry lobby_list[MAX_LOBBY_LIST];
static int lobby_count = 0;
static int selected_lobby = 0;

/* External Steam functions */
extern int steam_refresh_lobbies(void);
extern int steam_get_lobby_count(void);
extern const char* steam_get_lobby_name(int index);
extern uint64_t steam_get_lobby_id(int index);
extern int steam_get_lobby_players(int index);
extern int steam_join_lobby(uint64_t lobby_id);

/* Refresh the lobby list */
void refreshLobbyList(void) {
    printf("Refreshing lobby list...\n");
    steam_refresh_lobbies();
    
    /* Wait a bit for results (in real implementation, this should be async) */
    for (int i = 0; i < 10; i++) {
        extern void steam_update(void);
        steam_update();
    }
    
    /* Get lobby information */
    lobby_count = steam_get_lobby_count();
    if (lobby_count > MAX_LOBBY_LIST) {
        lobby_count = MAX_LOBBY_LIST;
    }
    
    for (int i = 0; i < lobby_count; i++) {
        lobby_list[i].lobby_id = steam_get_lobby_id(i);
        const char* name = steam_get_lobby_name(i);
        if (name) {
            strncpy(lobby_list[i].name, name, sizeof(lobby_list[i].name) - 1);
        } else {
            snprintf(lobby_list[i].name, sizeof(lobby_list[i].name), "Lobby %d", i + 1);
        }
        lobby_list[i].current_players = steam_get_lobby_players(i);
        lobby_list[i].max_players = 4;
        strcpy(lobby_list[i].host_name, "Unknown");
    }
    
    printf("Found %d lobbies\n", lobby_count);
}

/* Draw lobby browser */
void drawLobbyBrowser(gDisplay *d) {
    int x = d->vp_w / 6;
    int y = 2 * d->vp_h / 3;
    int size = d->vp_w / 32;
    if (size < 14) size = 14;
    int lineheight = size * 2;
    
    /* Title */
    glColor3f(1.0f, 1.0f, 0.0f);
    drawText(x, y + lineheight, size * 1.5, "MULTIPLAYER LOBBIES");
    
    /* Instructions */
    glColor3f(0.7f, 0.7f, 0.7f);
    drawText(x, y - lineheight * (lobby_count + 2), size * 0.8, 
             "UP/DOWN: Select  ENTER: Join  R: Refresh  ESC: Back");
    
    /* Draw lobby list */
    for (int i = 0; i < lobby_count; i++) {
        if (i == selected_lobby) {
            glColor3f(1.0f, 1.0f, 1.0f);  /* Highlight selected */
        } else {
            glColor3f(0.5f, 0.5f, 0.5f);  /* Dim others */
        }
        
        char lobby_text[128];
        snprintf(lobby_text, sizeof(lobby_text), "%s [%d/4 players]",
                lobby_list[i].name,
                lobby_list[i].current_players);
        
        drawText(x, y - i * lineheight, size, lobby_text);
    }
    
    /* No lobbies message */
    if (lobby_count == 0) {
        glColor3f(0.5f, 0.5f, 0.5f);
        drawText(x, y, size, "No lobbies found. Press R to refresh or C to create.");
    }
}

/* Handle lobby browser input */
int handleLobbyBrowserInput(unsigned char key) {
    switch(key) {
        case 'r':
        case 'R':
            refreshLobbyList();
            return 1;
            
        case 'c':
        case 'C':
            /* Create new lobby */
            extern void createLobby(void);
            createLobby();
            return 2;  /* Exit browser */
            
        case 13:  /* Enter */
            if (lobby_count > 0 && selected_lobby < lobby_count) {
                /* Join selected lobby */
                printf("Joining lobby: %s\n", lobby_list[selected_lobby].name);
                steam_join_lobby(lobby_list[selected_lobby].lobby_id);
                return 2;  /* Exit browser */
            }
            return 1;
            
        case 27:  /* ESC */
            return 0;  /* Exit browser */
    }
    return 1;  /* Continue browsing */
}

/* Handle special keys (arrows) */
void handleLobbyBrowserSpecial(int key) {
    switch(key) {
        case GLUT_KEY_UP:
            if (selected_lobby > 0) {
                selected_lobby--;
                playHighlightSound();
            }
            break;
            
        case GLUT_KEY_DOWN:
            if (selected_lobby < lobby_count - 1) {
                selected_lobby++;
                playHighlightSound();
            }
            break;
    }
}

/* Get selected lobby ID */
uint64_t getSelectedLobbyId(void) {
    if (selected_lobby >= 0 && selected_lobby < lobby_count) {
        return lobby_list[selected_lobby].lobby_id;
    }
    return 0;
}

#endif /* USE_STEAMWORKS */
