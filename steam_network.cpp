#ifdef USE_STEAMWORKS

#include "steam_multiplayer.h"
#include <string.h>

/* The functions are already declared in gltron.h which is included via steam_multiplayer.h */

/* P2P session request callback */
void SteamMultiplayer::OnP2PSessionRequest(P2PSessionRequest_t* pCallback)
{
    /* Accept P2P connection from lobby members */
    printf("P2P session request from player\n");
    SteamNetworking()->AcceptP2PSessionWithUser(pCallback->m_steamIDRemote);
}

/* Send packet to specific player */
void SteamMultiplayer::SendPacket(NetworkPacket* packet, CSteamID target, bool reliable)
{
    if (!m_bInLobby) return;
    
    EP2PSend sendType = reliable ? k_EP2PSendReliable : k_EP2PSendUnreliable;
    
    SteamNetworking()->SendP2PPacket(
        target,
        packet,
        sizeof(NetworkPacket),
        sendType,
        0  /* Channel 0 */
    );
}

/* Broadcast packet to all players */
void SteamMultiplayer::BroadcastPacket(NetworkPacket* packet, bool reliable)
{
    if (!m_bInLobby) return;
    
    for (int i = 0; i < m_nPlayers; i++) {
        if (m_PlayerIDs[i] != SteamUser()->GetSteamID()) {
            SendPacket(packet, m_PlayerIDs[i], reliable);
        }
    }
}

/* Receive packet */
bool SteamMultiplayer::ReceivePacket(NetworkPacket* packet, CSteamID* sender)
{
    uint32 msgSize = 0;
    
    /* Check if packet is available */
    if (!SteamNetworking()->IsP2PPacketAvailable(&msgSize, 0)) {
        return false;
    }
    
    if (msgSize != sizeof(NetworkPacket)) {
        /* Wrong packet size, discard */
        char buffer[1024];
        uint32 bytesRead;
        SteamNetworking()->ReadP2PPacket(buffer, sizeof(buffer), &bytesRead, sender, 0);
        return false;
    }
    
    uint32 bytesRead;
    if (SteamNetworking()->ReadP2PPacket(packet, sizeof(NetworkPacket), &bytesRead, sender, 0)) {
        return true;
    }
    
    return false;
}

/* Process all pending network messages */
void SteamMultiplayer::ProcessNetworkMessages()
{
    if (!m_bInLobby) return;
    
    NetworkPacket packet;
    CSteamID sender;
    
    while (ReceivePacket(&packet, &sender)) {
        /* Process packet based on type */
        switch (packet.type) {
            case MSG_PLAYER_UPDATE:
                /* Update remote player position */
                if (game && packet.player_id >= 0 && packet.player_id < MAX_PLAYERS) {
                    Data* data = game->player[packet.player_id].data;
                    if (data) {
                        data->posx = packet.posx;
                        data->posy = packet.posy;
                        data->dir = packet.dir;
                        data->speed = packet.speed;
                    }
                }
                break;
                
            case MSG_PLAYER_TURN:
                /* Handle remote player turn */
                if (game && packet.player_id >= 0 && packet.player_id < MAX_PLAYERS) {
                    Data* data = game->player[packet.player_id].data;
                    if (data) {
                        turn(data, packet.dir);
                    }
                }
                break;
                
            case MSG_PLAYER_CRASH:
                /* Handle remote player crash */
                if (game && packet.player_id >= 0 && packet.player_id < MAX_PLAYERS) {
                    Data* data = game->player[packet.player_id].data;
                    if (data) {
                        data->speed = SPEED_CRASHED;
                    }
                }
                break;
                
            case MSG_GAME_START:
                /* Start game for clients */
                if (!m_bIsHost && game) {
                    printf("Received game start signal from host\n");
                    
                    /* Setup players - packet.player_id contains our assigned slot */
                    game->players = 4;
                    
                    /* Find our player slot (sent by host) */
                    int my_slot = packet.player_id;
                    if (my_slot < 0 || my_slot >= 4) my_slot = 1;  /* Default to player 1 */
                    
                    /* Configure all players */
                    for (int i = 0; i < 4; i++) {
                        if (i == my_slot) {
                            game->player[i].ai->active = -1;  /* We control this player */
                        } else if (i < m_nPlayers) {
                            game->player[i].ai->active = 2;   /* Remote player */
                        } else {
                            game->player[i].ai->active = 1;   /* AI player */
                        }
                    }
                    
                    /* Initialize and start */
                    initData();
                    switchCallbacks(&gameCallbacks);
                }
                break;
                
            case MSG_GAME_END:
                /* End game */
                game->winner = packet.player_id;
                game->pauseflag = PAUSE_GAME_FINISHED;
                switchCallbacks(&pauseCallbacks);
                break;
                
            case MSG_CHAT:
                /* Display chat message */
                printf("Chat: %s\n", packet.data);
                break;
                
            default:
                break;
        }
    }
}

/* Send player update */
void SteamMultiplayer::SendPlayerUpdate(int player_id, float x, float y, int dir, float speed)
{
    NetworkPacket packet;
    packet.type = MSG_PLAYER_UPDATE;
    packet.player_id = player_id;
    packet.posx = x;
    packet.posy = y;
    packet.dir = dir;
    packet.speed = speed;
    packet.timestamp = getElapsedTime();
    
    BroadcastPacket(&packet, false);  /* Unreliable for position updates */
}

/* Send player turn */
void SteamMultiplayer::SendPlayerTurn(int player_id, int direction)
{
    NetworkPacket packet;
    packet.type = MSG_PLAYER_TURN;
    packet.player_id = player_id;
    packet.dir = direction;
    packet.timestamp = getElapsedTime();
    
    BroadcastPacket(&packet, true);  /* Reliable for turns */
}

/* Send player crash */
void SteamMultiplayer::SendPlayerCrash(int player_id)
{
    NetworkPacket packet;
    packet.type = MSG_PLAYER_CRASH;
    packet.player_id = player_id;
    packet.timestamp = getElapsedTime();
    
    BroadcastPacket(&packet, true);  /* Reliable for crashes */
}

/* Send game start */
void SteamMultiplayer::SendGameStart()
{
    /* Send individual start messages to each player with their slot assignment */
    for (int i = 0; i < m_nPlayers; i++) {
        if (m_PlayerIDs[i] != SteamUser()->GetSteamID()) {
            NetworkPacket packet;
            packet.type = MSG_GAME_START;
            packet.player_id = i;  /* Assign this player to slot i */
            packet.timestamp = getElapsedTime();
            
            printf("Sending game start to player %d (slot %d)\n", i, i);
            SendPacket(&packet, m_PlayerIDs[i], true);
        }
    }
}

/* Send game end */
void SteamMultiplayer::SendGameEnd(int winner)
{
    NetworkPacket packet;
    packet.type = MSG_GAME_END;
    packet.player_id = winner;
    packet.timestamp = getElapsedTime();
    
    BroadcastPacket(&packet, true);
}

#endif /* USE_STEAMWORKS */
