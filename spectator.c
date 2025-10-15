#include "gltron.h"
#include <stdio.h>

/* Spectator mode state */
typedef struct {
    int active;           /* Is spectator mode active? */
    int target_player;    /* Which player are we spectating? */
    int local_player;     /* Our original player index */
    int can_switch;       /* Can switch between players? */
} SpectatorState;

static SpectatorState spectator = {0, 0, 0, 1};

/* Enter spectator mode */
void enterSpectatorMode(int player_index) {
    if (spectator.active) return;
    
    printf("Entering spectator mode for player %d\n", player_index);
    
    spectator.active = 1;
    spectator.local_player = player_index;
    spectator.target_player = -1;
    
    /* Find first alive non-AI player to spectate */
    for (int i = 0; i < game->players; i++) {
        if (i != player_index && 
            game->player[i].data->speed > 0 &&
            game->player[i].ai->active != 1) {  /* Not AI */
            spectator.target_player = i;
            break;
        }
    }
    
    /* If no human players alive, spectate any alive player */
    if (spectator.target_player == -1) {
        for (int i = 0; i < game->players; i++) {
            if (i != player_index && game->player[i].data->speed > 0) {
                spectator.target_player = i;
                break;
            }
        }
    }
    
    if (spectator.target_player >= 0) {
        printf("Now spectating player %d\n", spectator.target_player);
    }
}

/* Exit spectator mode */
void exitSpectatorMode(void) {
    if (!spectator.active) return;
    
    printf("Exiting spectator mode\n");
    spectator.active = 0;
    spectator.target_player = 0;
}

/* Switch to next player */
void spectatorNextPlayer(void) {
    if (!spectator.active || !spectator.can_switch) return;
    
    int original = spectator.target_player;
    int next = (spectator.target_player + 1) % game->players;
    
    /* Find next alive non-AI player */
    while (next != original) {
        if (next != spectator.local_player && 
            game->player[next].data->speed > 0 &&
            game->player[next].ai->active != 1) {  /* Prefer non-AI */
            spectator.target_player = next;
            printf("Spectating player %d\n", next);
            return;
        }
        next = (next + 1) % game->players;
    }
}

/* Switch to previous player */
void spectatorPrevPlayer(void) {
    if (!spectator.active || !spectator.can_switch) return;
    
    int original = spectator.target_player;
    int prev = (spectator.target_player - 1 + game->players) % game->players;
    
    /* Find previous alive non-AI player */
    while (prev != original) {
        if (prev != spectator.local_player && 
            game->player[prev].data->speed > 0 &&
            game->player[prev].ai->active != 1) {  /* Prefer non-AI */
            spectator.target_player = prev;
            printf("Spectating player %d\n", prev);
            return;
        }
        prev = (prev - 1 + game->players) % game->players;
    }
}

/* Get current spectator target */
int getSpectatorTarget(void) {
    if (spectator.active && spectator.target_player >= 0) {
        return spectator.target_player;
    }
    return -1;
}

/* Check if in spectator mode */
int isSpectatorMode(void) {
    return spectator.active;
}

/* Update spectator camera */
void updateSpectatorCamera(void) {
    if (!spectator.active || spectator.target_player < 0) return;
    
    /* Check if target player is still alive */
    if (game->player[spectator.target_player].data->speed <= 0) {
        /* Target died, find another player */
        spectatorNextPlayer();
    }
    
    /* If we have a valid target, update our camera to follow them */
    if (spectator.target_player >= 0 && spectator.target_player < game->players) {
        Camera *our_cam = game->player[spectator.local_player].camera;
        Camera *target_cam = game->player[spectator.target_player].camera;
        Data *target_data = game->player[spectator.target_player].data;
        
        if (our_cam && target_cam && target_data) {
            /* Copy camera position from target */
            our_cam->cam[0] = target_cam->cam[0];
            our_cam->cam[1] = target_cam->cam[1];
            our_cam->cam[2] = target_cam->cam[2];
            
            our_cam->target[0] = target_data->posx;
            our_cam->target[1] = target_data->posy;
            our_cam->target[2] = 0;
            
            our_cam->camType = target_cam->camType;
        }
    }
}

/* Draw spectator UI */
void drawSpectatorUI(gDisplay *d) {
    if (!spectator.active) return;
    
    int size = d->vp_w / 40;
    if (size < 12) size = 12;
    
    /* Draw "SPECTATING" text */
    glColor4f(1.0f, 1.0f, 1.0f, 0.8f);
    drawText(d->vp_w / 2 - size * 5, d->vp_h - size * 2, size, "SPECTATING");
    
    /* Draw player name/number */
    if (spectator.target_player >= 0) {
        char buf[64];
        const char* player_type = "";
        
        if (game->player[spectator.target_player].ai->active == -1) {
            player_type = "Human";
        } else if (game->player[spectator.target_player].ai->active == 2) {
            player_type = "Remote";
        } else {
            player_type = "AI";
        }
        
        snprintf(buf, sizeof(buf), "Player %d (%s)", 
                 spectator.target_player + 1, player_type);
        drawText(d->vp_w / 2 - size * 6, d->vp_h - size * 4, size, buf);
    }
    
    /* Draw controls hint */
    glColor4f(0.7f, 0.7f, 0.7f, 0.6f);
    drawText(d->vp_w / 2 - size * 8, size * 2, size * 0.8, 
             "LEFT/RIGHT: Switch Player  ESC: Menu");
}
