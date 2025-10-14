#ifdef USE_STEAMWORKS

#include "steam_multiplayer.h"

/* C interface implementation */
extern "C" {

/* Initialize Steam */
int steam_init(void)
{
    if (g_pSteamMultiplayer) {
        return 1;  /* Already initialized */
    }
    
    g_pSteamMultiplayer = new SteamMultiplayer();
    if (!g_pSteamMultiplayer->Initialize()) {
        delete g_pSteamMultiplayer;
        g_pSteamMultiplayer = nullptr;
        return 0;
    }
    
    return 1;
}

/* Shutdown Steam */
void steam_shutdown(void)
{
    if (g_pSteamMultiplayer) {
        g_pSteamMultiplayer->Shutdown();
        delete g_pSteamMultiplayer;
        g_pSteamMultiplayer = nullptr;
    }
}

/* Update Steam (run callbacks) */
void steam_update(void)
{
    if (g_pSteamMultiplayer) {
        g_pSteamMultiplayer->RunCallbacks();
        g_pSteamMultiplayer->ProcessNetworkMessages();
    }
}

/* Create lobby */
int steam_create_lobby(const char* name, int max_players)
{
    if (!g_pSteamMultiplayer) return 0;
    g_pSteamMultiplayer->CreateLobby(name, max_players);
    return 1;
}

/* Join lobby */
int steam_join_lobby(uint64_t lobby_id)
{
    if (!g_pSteamMultiplayer) return 0;
    CSteamID steamID;
    steamID.SetFromUint64(lobby_id);
    g_pSteamMultiplayer->JoinLobby(steamID);
    return 1;
}

/* Leave lobby */
void steam_leave_lobby(void)
{
    if (g_pSteamMultiplayer) {
        g_pSteamMultiplayer->LeaveLobby();
    }
}

/* Refresh lobby list */
int steam_refresh_lobbies(void)
{
    if (!g_pSteamMultiplayer) return 0;
    g_pSteamMultiplayer->RefreshLobbyList();
    return 1;
}

/* Send player update */
void steam_send_player_update(int player, float x, float y, int dir, float speed)
{
    if (g_pSteamMultiplayer && g_pSteamMultiplayer->IsInLobby()) {
        g_pSteamMultiplayer->SendPlayerUpdate(player, x, y, dir, speed);
    }
}

/* Send player turn */
void steam_send_player_turn(int player, int direction)
{
    if (g_pSteamMultiplayer && g_pSteamMultiplayer->IsInLobby()) {
        g_pSteamMultiplayer->SendPlayerTurn(player, direction);
    }
}

/* Send player crash */
void steam_send_player_crash(int player)
{
    if (g_pSteamMultiplayer && g_pSteamMultiplayer->IsInLobby()) {
        g_pSteamMultiplayer->SendPlayerCrash(player);
    }
}

/* Send game start */
void steam_send_game_start(void)
{
    if (g_pSteamMultiplayer && g_pSteamMultiplayer->IsInLobby()) {
        g_pSteamMultiplayer->SendGameStart();
    }
}

/* Send game end */
void steam_send_game_end(int winner)
{
    if (g_pSteamMultiplayer && g_pSteamMultiplayer->IsInLobby()) {
        g_pSteamMultiplayer->SendGameEnd(winner);
    }
}

/* Send chat message */
void steam_send_chat(const char* message)
{
    if (g_pSteamMultiplayer && g_pSteamMultiplayer->IsInLobby()) {
        g_pSteamMultiplayer->SendChatMessage(message);
    }
}

/* Check if in multiplayer */
int steam_is_multiplayer(void)
{
    return g_pSteamMultiplayer && g_pSteamMultiplayer->IsInLobby();
}

/* Check if host */
int steam_is_host(void)
{
    return g_pSteamMultiplayer && g_pSteamMultiplayer->IsHost();
}

/* Get player count */
int steam_get_player_count(void)
{
    if (g_pSteamMultiplayer && g_pSteamMultiplayer->IsInLobby()) {
        return g_pSteamMultiplayer->GetNumPlayers();
    }
    return 0;
}

} /* extern "C" */

#endif /* USE_STEAMWORKS */
