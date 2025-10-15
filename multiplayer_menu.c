#include "gltron.h"
#include <string.h>
#include <stdio.h>

#ifdef USE_STEAMWORKS
/* Steam multiplayer C interface */
extern int steam_init(void);
extern void steam_shutdown(void);
extern void steam_update(void);
extern int steam_create_lobby(const char* name, int max_players);
extern int steam_join_lobby(uint64_t lobby_id);
extern void steam_leave_lobby(void);
extern int steam_refresh_lobbies(void);
extern int steam_is_multiplayer(void);
extern int steam_is_host(void);
extern int steam_get_player_count(void);
#endif

/* Multiplayer menu state */
typedef struct {
    int initialized;
    int in_lobby;
    int is_host;
    int player_count;
    char lobby_name[64];
    char status_message[128];
} MultiplayerState;

static MultiplayerState mp_state = {0};

/* Initialize multiplayer */
void initMultiplayer(void) {
#ifdef USE_STEAMWORKS
    if (!mp_state.initialized) {
        printf("Initializing Steam multiplayer...\n");
        if (steam_init()) {
            mp_state.initialized = 1;
            strcpy(mp_state.status_message, "Steam connected");
            printf("Steam multiplayer initialized\n");
        } else {
            strcpy(mp_state.status_message, "Steam connection failed");
            printf("Failed to initialize Steam\n");
        }
    }
#else
    strcpy(mp_state.status_message, "Multiplayer not available (no Steam)");
#endif
}

/* Shutdown multiplayer */
void shutdownMultiplayer(void) {
#ifdef USE_STEAMWORKS
    if (mp_state.initialized) {
        steam_shutdown();
        mp_state.initialized = 0;
        mp_state.in_lobby = 0;
        mp_state.is_host = 0;
    }
#endif
}

/* Update multiplayer */
void updateMultiplayer(void) {
#ifdef USE_STEAMWORKS
    if (mp_state.initialized) {
        steam_update();
        
        /* Update state */
        mp_state.in_lobby = steam_is_multiplayer();
        mp_state.is_host = steam_is_host();
        mp_state.player_count = steam_get_player_count();
        
        if (mp_state.in_lobby) {
            snprintf(mp_state.status_message, sizeof(mp_state.status_message),
                    "In lobby (%d players) - %s", 
                    mp_state.player_count,
                    mp_state.is_host ? "Host" : "Client");
        }
    }
#endif
}

/* Create a new lobby */
void createLobby(void) {
#ifdef USE_STEAMWORKS
    if (mp_state.initialized && !mp_state.in_lobby) {
        snprintf(mp_state.lobby_name, sizeof(mp_state.lobby_name),
                "GLTron Game %d", getElapsedTime());
        
        if (steam_create_lobby(mp_state.lobby_name, 4)) {
            strcpy(mp_state.status_message, "Creating lobby...");
            
            /* Auto-start with AI if we're the host */
            mp_state.in_lobby = 1;
            mp_state.is_host = 1;
            mp_state.player_count = 1;
        } else {
            strcpy(mp_state.status_message, "Failed to create lobby");
        }
    }
#endif
}

/* Quick match - join existing or create new */
void quickMatch(void) {
#ifdef USE_STEAMWORKS
    if (!mp_state.initialized) {
        initMultiplayer();
    }
    
    if (mp_state.initialized && !mp_state.in_lobby) {
        printf("Quick match: searching for lobbies...\n");
        
        /* Try to refresh and join an existing lobby */
        refreshLobbies();
        
        /* Wait for lobby list */
        for (int i = 0; i < 20; i++) {
            updateMultiplayer();
        }
        
        /* Check if any lobbies found */
        int lobby_count = steam_get_lobby_count();
        if (lobby_count > 0) {
            /* Join first available lobby */
            uint64_t lobby_id = steam_get_lobby_id(0);
            if (lobby_id) {
                printf("Joining existing lobby\n");
                joinLobby(lobby_id);
                return;
            }
        }
        
        /* No lobbies found, create a new one */
        printf("No lobbies found, creating new lobby\n");
        createLobby();
        
        /* If we created a lobby, start game with AI after a short delay */
        if (mp_state.is_host) {
            /* Give time for others to join */
            printf("Starting game with AI players...\n");
            startMultiplayerGame();
        }
    }
#endif
}

/* Join a lobby */
void joinLobby(uint64_t lobby_id) {
#ifdef USE_STEAMWORKS
    if (mp_state.initialized && !mp_state.in_lobby) {
        if (steam_join_lobby(lobby_id)) {
            strcpy(mp_state.status_message, "Joining lobby...");
        } else {
            strcpy(mp_state.status_message, "Failed to join lobby");
        }
    }
#endif
}

/* Leave current lobby */
void leaveLobby(void) {
#ifdef USE_STEAMWORKS
    if (mp_state.initialized && mp_state.in_lobby) {
        steam_leave_lobby();
        mp_state.in_lobby = 0;
        mp_state.is_host = 0;
        strcpy(mp_state.status_message, "Left lobby");
    }
#endif
}

/* Refresh lobby list */
void refreshLobbies(void) {
#ifdef USE_STEAMWORKS
    if (mp_state.initialized && !mp_state.in_lobby) {
        steam_refresh_lobbies();
        strcpy(mp_state.status_message, "Refreshing lobbies...");
    }
#endif
}

/* Start multiplayer game */
void startMultiplayerGame(void) {
#ifdef USE_STEAMWORKS
    if (mp_state.in_lobby && mp_state.is_host) {
        printf("Starting multiplayer game with %d players\n", mp_state.player_count);
        
        /* Always use 4 players in multiplayer */
        game->players = 4;
        
        /* Configure players based on who's in the lobby */
        /* Player 0 is always the local player (host) */
        game->player[0].ai->active = -1;  /* -1 = human player */
        
        /* Players 1 to player_count-1 are remote players */
        for (int i = 1; i < mp_state.player_count && i < 4; i++) {
            game->player[i].ai->active = 2;  /* 2 = remote player */
            printf("Player %d set as remote player\n", i);
        }
        
        /* Remaining slots are AI */
        for (int i = mp_state.player_count; i < 4; i++) {
            game->player[i].ai->active = 1;  /* 1 = AI player */
            printf("Player %d set as AI\n", i);
        }
        
        /* Initialize game data */
        initData();
        
        /* Notify other players to start */
        steam_send_game_start();
        
        /* Switch to game */
        switchCallbacks(&gameCallbacks);
    }
#endif
}

/* Get multiplayer status */
const char* getMultiplayerStatus(void) {
    return mp_state.status_message;
}

/* Check if in multiplayer mode */
int isMultiplayer(void) {
    return mp_state.in_lobby;
}

/* Check if host */
int isMultiplayerHost(void) {
    return mp_state.is_host;
}
