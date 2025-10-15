#ifndef STEAM_MULTIPLAYER_H
#define STEAM_MULTIPLAYER_H

#ifdef USE_STEAMWORKS

#include "steam/steam_api.h"
#include "gltron.h"

/* Steam App ID - Replace with your actual App ID */
#define GLTRON_STEAM_APP_ID 480  /* Use 480 for testing (Spacewar) */

/* Network message types */
typedef enum {
    MSG_PLAYER_UPDATE = 1,
    MSG_PLAYER_TURN,
    MSG_PLAYER_CRASH,
    MSG_GAME_START,
    MSG_GAME_END,
    MSG_CHAT,
    MSG_PING,
    MSG_LOBBY_INFO
} MessageType;

/* Network packet structure */
typedef struct {
    MessageType type;
    int player_id;
    float posx, posy;
    int dir;
    float speed;
    int timestamp;
    char data[256];  /* Additional data */
} NetworkPacket;

/* Lobby info */
typedef struct {
    CSteamID lobby_id;
    CSteamID owner_id;
    int num_players;
    int max_players;
    char lobby_name[64];
    bool is_host;
    bool in_game;
} LobbyInfo;

/* Steam multiplayer manager */
class SteamMultiplayer {
private:
    bool m_bInitialized;
    bool m_bInLobby;
    bool m_bIsHost;
    CSteamID m_LobbyID;
    CSteamID m_PlayerIDs[MAX_PLAYERS];
    int m_nPlayers;
    
    /* Steam callbacks */
    STEAM_CALLBACK(SteamMultiplayer, OnLobbyCreated, LobbyCreated_t);
    STEAM_CALLBACK(SteamMultiplayer, OnLobbyEnter, LobbyEnter_t);
    STEAM_CALLBACK(SteamMultiplayer, OnLobbyChatUpdate, LobbyChatUpdate_t);
    STEAM_CALLBACK(SteamMultiplayer, OnLobbyDataUpdate, LobbyDataUpdate_t);
    STEAM_CALLBACK(SteamMultiplayer, OnP2PSessionRequest, P2PSessionRequest_t);
    /* OnLobbyMatchList is a CCallResult, not a STEAM_CALLBACK */
    void OnLobbyMatchList(LobbyMatchList_t* pCallback, bool bIOFailure);
    
    CCallResult<SteamMultiplayer, LobbyMatchList_t> m_LobbyMatchListResult;
    
public:
    SteamMultiplayer();
    ~SteamMultiplayer();
    
    /* Initialization */
    bool Initialize();
    void Shutdown();
    void RunCallbacks();
    
    /* Lobby management */
    void CreateLobby(const char* lobby_name, int max_players);
    void JoinLobby(CSteamID lobby_id);
    void LeaveLobby();
    void RefreshLobbyList();
    void SendChatMessage(const char* message);
    
    /* Game networking */
    void SendPacket(NetworkPacket* packet, CSteamID target, bool reliable = true);
    void BroadcastPacket(NetworkPacket* packet, bool reliable = true);
    bool ReceivePacket(NetworkPacket* packet, CSteamID* sender);
    void ProcessNetworkMessages();
    
    /* Player synchronization */
    void SendPlayerUpdate(int player_id, float x, float y, int dir, float speed);
    void SendPlayerTurn(int player_id, int direction);
    void SendPlayerCrash(int player_id);
    void SendGameStart();
    void SendGameEnd(int winner);
    
    /* Getters */
    bool IsInitialized() const { return m_bInitialized; }
    bool IsInLobby() const { return m_bInLobby; }
    bool IsHost() const { return m_bIsHost; }
    int GetNumPlayers() const { return m_nPlayers; }
    CSteamID GetLobbyID() const { return m_LobbyID; }
    CSteamID GetPlayerID(int index) const { return m_PlayerIDs[index]; }
};

/* Global Steam multiplayer instance */
extern SteamMultiplayer* g_pSteamMultiplayer;

/* C interface for integration with existing code */
#ifdef __cplusplus
extern "C" {
#endif

int steam_init(void);
void steam_shutdown(void);
void steam_update(void);

/* Lobby functions */
int steam_create_lobby(const char* name, int max_players);
int steam_join_lobby(uint64_t lobby_id);
void steam_leave_lobby(void);
int steam_refresh_lobbies(void);
int steam_get_lobby_count(void);
const char* steam_get_lobby_name(int index);
uint64_t steam_get_lobby_id(int index);
int steam_get_lobby_players(int index);

/* Network functions */
void steam_send_player_update(int player, float x, float y, int dir, float speed);
void steam_send_player_turn(int player, int direction);
void steam_send_player_crash(int player);
void steam_send_game_start(void);
void steam_send_game_end(int winner);
void steam_send_chat(const char* message);

/* Status functions */
int steam_is_multiplayer(void);
int steam_is_host(void);
int steam_get_player_count(void);

#ifdef __cplusplus
}
#endif

#endif /* USE_STEAMWORKS */

#endif /* STEAM_MULTIPLAYER_H */
