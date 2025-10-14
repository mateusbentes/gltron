#ifdef USE_STEAMWORKS

#include "steam_multiplayer.h"
#include <stdio.h>
#include <string.h>

/* Global instance */
SteamMultiplayer* g_pSteamMultiplayer = nullptr;

/* Constructor */
SteamMultiplayer::SteamMultiplayer() :
    m_bInitialized(false),
    m_bInLobby(false),
    m_bIsHost(false),
    m_nPlayers(0)
{
    memset(m_PlayerIDs, 0, sizeof(m_PlayerIDs));
}

/* Destructor */
SteamMultiplayer::~SteamMultiplayer()
{
    if (m_bInLobby) {
        LeaveLobby();
    }
}

/* Initialize Steam */
bool SteamMultiplayer::Initialize()
{
    if (m_bInitialized) {
        return true;
    }
    
    /* Initialize Steam API */
    if (!SteamAPI_Init()) {
        printf("Failed to initialize Steam API\n");
        return false;
    }
    
    /* Check if user is logged in */
    if (!SteamUser()->BLoggedOn()) {
        printf("Steam user is not logged in\n");
        SteamAPI_Shutdown();
        return false;
    }
    
    printf("Steam initialized successfully\n");
    printf("Steam User: %s\n", SteamFriends()->GetPersonaName());
    
    m_bInitialized = true;
    return true;
}

/* Shutdown Steam */
void SteamMultiplayer::Shutdown()
{
    if (m_bInitialized) {
        if (m_bInLobby) {
            LeaveLobby();
        }
        SteamAPI_Shutdown();
        m_bInitialized = false;
    }
}

/* Run Steam callbacks */
void SteamMultiplayer::RunCallbacks()
{
    if (m_bInitialized) {
        SteamAPI_RunCallbacks();
    }
}

#endif /* USE_STEAMWORKS */
