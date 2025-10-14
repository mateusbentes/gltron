#ifdef USE_STEAMWORKS

#include "steam_multiplayer.h"
#include <stdio.h>

/* Create a lobby */
void SteamMultiplayer::CreateLobby(const char* lobby_name, int max_players)
{
    if (!m_bInitialized) return;
    
    if (m_bInLobby) {
        LeaveLobby();
    }
    
    printf("Creating lobby: %s (max %d players)\n", lobby_name, max_players);
    
    /* Create public lobby */
    SteamAPICall_t hCall = SteamMatchmaking()->CreateLobby(
        k_ELobbyTypePublic,  /* Public lobby */
        max_players
    );
}

/* Join a lobby */
void SteamMultiplayer::JoinLobby(CSteamID lobby_id)
{
    if (!m_bInitialized) return;
    
    if (m_bInLobby) {
        LeaveLobby();
    }
    
    printf("Joining lobby...\n");
    SteamMatchmaking()->JoinLobby(lobby_id);
}

/* Leave current lobby */
void SteamMultiplayer::LeaveLobby()
{
    if (!m_bInLobby) return;
    
    printf("Leaving lobby\n");
    
    /* Close P2P sessions with all players */
    for (int i = 0; i < m_nPlayers; i++) {
        if (m_PlayerIDs[i].IsValid()) {
            SteamNetworking()->CloseP2PSessionWithUser(m_PlayerIDs[i]);
        }
    }
    
    SteamMatchmaking()->LeaveLobby(m_LobbyID);
    
    m_bInLobby = false;
    m_bIsHost = false;
    m_nPlayers = 0;
    m_LobbyID = CSteamID();
}

/* Refresh lobby list */
void SteamMultiplayer::RefreshLobbyList()
{
    if (!m_bInitialized) return;
    
    printf("Refreshing lobby list...\n");
    
    /* Add filters for GLTron lobbies */
    SteamMatchmaking()->AddRequestLobbyListStringFilter(
        "game", "gltron", k_ELobbyComparisonEqual
    );
    
    /* Request lobby list */
    SteamAPICall_t hCall = SteamMatchmaking()->RequestLobbyList();
    m_LobbyMatchListResult.Set(hCall, this, &SteamMultiplayer::OnLobbyMatchList);
}

/* Send chat message */
void SteamMultiplayer::SendChatMessage(const char* message)
{
    if (!m_bInLobby) return;
    
    SteamMatchmaking()->SendLobbyChatMsg(
        m_LobbyID,
        message,
        strlen(message) + 1
    );
}

/* Lobby created callback */
void SteamMultiplayer::OnLobbyCreated(LobbyCreated_t* pCallback)
{
    if (pCallback->m_eResult != k_EResultOK) {
        printf("Failed to create lobby\n");
        return;
    }
    
    m_LobbyID = pCallback->m_ulSteamIDLobby;
    m_bInLobby = true;
    m_bIsHost = true;
    
    /* Set lobby data */
    SteamMatchmaking()->SetLobbyData(m_LobbyID, "game", "gltron");
    SteamMatchmaking()->SetLobbyData(m_LobbyID, "version", "1.0");
    SteamMatchmaking()->SetLobbyData(m_LobbyID, "host", 
        SteamFriends()->GetPersonaName());
    
    printf("Lobby created successfully\n");
}

/* Lobby enter callback */
void SteamMultiplayer::OnLobbyEnter(LobbyEnter_t* pCallback)
{
    if (pCallback->m_EChatRoomEnterResponse != k_EChatRoomEnterResponseSuccess) {
        printf("Failed to enter lobby\n");
        return;
    }
    
    m_LobbyID = pCallback->m_ulSteamIDLobby;
    m_bInLobby = true;
    
    /* Check if we are the host */
    CSteamID owner = SteamMatchmaking()->GetLobbyOwner(m_LobbyID);
    m_bIsHost = (owner == SteamUser()->GetSteamID());
    
    /* Get player list */
    m_nPlayers = SteamMatchmaking()->GetNumLobbyMembers(m_LobbyID);
    for (int i = 0; i < m_nPlayers; i++) {
        m_PlayerIDs[i] = SteamMatchmaking()->GetLobbyMemberByIndex(m_LobbyID, i);
    }
    
    printf("Entered lobby (Host: %s, Players: %d)\n", 
           m_bIsHost ? "Yes" : "No", m_nPlayers);
}

/* Lobby chat update callback */
void SteamMultiplayer::OnLobbyChatUpdate(LobbyChatUpdate_t* pCallback)
{
    /* Handle player join/leave */
    if (pCallback->m_rgfChatMemberStateChange & k_EChatMemberStateChangeEntered) {
        printf("Player joined lobby\n");
        /* Update player list */
        m_nPlayers = SteamMatchmaking()->GetNumLobbyMembers(m_LobbyID);
        for (int i = 0; i < m_nPlayers; i++) {
            m_PlayerIDs[i] = SteamMatchmaking()->GetLobbyMemberByIndex(m_LobbyID, i);
        }
    }
    
    if (pCallback->m_rgfChatMemberStateChange & k_EChatMemberStateChangeLeft) {
        printf("Player left lobby\n");
        /* Update player list */
        m_nPlayers = SteamMatchmaking()->GetNumLobbyMembers(m_LobbyID);
        for (int i = 0; i < m_nPlayers; i++) {
            m_PlayerIDs[i] = SteamMatchmaking()->GetLobbyMemberByIndex(m_LobbyID, i);
        }
    }
}

/* Lobby data update callback */
void SteamMultiplayer::OnLobbyDataUpdate(LobbyDataUpdate_t* pCallback)
{
    /* Handle lobby data changes */
}

/* Lobby match list callback */
void SteamMultiplayer::OnLobbyMatchList(LobbyMatchList_t* pCallback, bool bIOFailure)
{
    if (bIOFailure) {
        printf("Failed to get lobby list\n");
        return;
    }
    
    printf("Found %d lobbies\n", pCallback->m_nLobbiesMatching);
}

#endif /* USE_STEAMWORKS */
