# GLTron Steam Multiplayer

## Overview

GLTron now supports online multiplayer through Steam! Play with up to 4 players in peer-to-peer matches using Steam's networking infrastructure.

## Features

- **Steam Lobby System**: Create and join game lobbies through Steam
- **Peer-to-Peer Networking**: Direct connections between players for low latency
- **Automatic Synchronization**: Player positions, turns, and crashes are synchronized
- **Host Migration**: If the host leaves, another player becomes the host
- **Steam Friends Integration**: See and join friends' games

## Building with Steam Support

### Prerequisites

1. **Steamworks SDK**: Download from [Steam Partner](https://partner.steamgames.com/)
2. **Steam Client**: Must be running when playing
3. **Steam Account**: Required for multiplayer

### Build Instructions

1. Place Steamworks SDK in `gltron/steamworks/` directory
2. Enable Steam support in CMake:
   ```bash
   cd build
   cmake .. -DUSE_STEAMWORKS=ON
   make
   ```

3. Copy `steam_appid.txt` to the build directory (contains App ID 480 for testing)

## How to Play Multiplayer

### Creating a Lobby

1. Start GLTron with Steam running
2. Go to Main Menu → Multiplayer
3. Select "Create Lobby"
4. Wait for other players to join
5. Select "Start Multiplayer Game" when ready

### Joining a Lobby

1. Start GLTron with Steam running
2. Go to Main Menu → Multiplayer
3. Select "Refresh Lobbies" to see available games
4. Select "Join Lobby" to join a game
5. Wait for the host to start the game

### In-Game

- The game plays like normal GLTron
- All players start simultaneously
- Crashes and positions are synchronized
- The last player alive wins
- Press ESC to return to menu

## Network Architecture

### Peer-to-Peer Model

- No dedicated server required
- Host player manages game state
- All players connect directly to each other
- Steam handles NAT traversal and connection management

### Message Types

- **Player Updates**: Position and direction (unreliable, frequent)
- **Player Turns**: Turn events (reliable)
- **Player Crashes**: Crash events (reliable)
- **Game Start/End**: Game state changes (reliable)

### Synchronization

- Position updates sent 20 times per second
- Turn events sent immediately
- Crash detection synchronized across all clients
- Game state managed by host

## Troubleshooting

### Steam Not Detected

- Ensure Steam is running before starting GLTron
- Check that steam_appid.txt is in the same directory as the executable
- Verify Steamworks library is properly linked

### Cannot Create/Join Lobbies

- Check your Steam network settings
- Ensure you're online in Steam
- Verify firewall isn't blocking Steam

### Lag or Desync Issues

- Check your internet connection
- Ensure all players have good connections
- Host should have the best connection
- Try creating a new lobby

### Build Issues

- Verify Steamworks SDK is in the correct location
- Check that CMAKE finds the Steam libraries
- Ensure you're building with C++ support (required for Steam)

## Technical Details

### Files Added

- `steam_multiplayer.h/cpp`: Main Steam integration
- `steam_lobby.cpp`: Lobby management
- `steam_network.cpp`: Networking code
- `steam_interface.cpp`: C interface for integration
- `multiplayer_menu.c`: Menu integration

### Network Protocol

- Uses Steam P2P networking API
- Reliable and unreliable channels
- Automatic packet fragmentation
- Built-in encryption and authentication

### Limitations

- Maximum 4 players (GLTron game limit)
- Requires Steam client running
- Uses test App ID 480 (Spacewar) for development

## Future Improvements

- Spectator mode
- Tournament/bracket system
- Replay system
- Custom lobby settings
- Chat system
- Friends list integration
- Achievements and leaderboards

## Development Notes

For production use, you'll need:
1. Your own Steam App ID
2. Proper Steam integration on Steam store
3. Update GLTRON_STEAM_APP_ID in steam_multiplayer.h

## License

The Steam integration follows the Steamworks SDK license.
GLTron remains under its original license.
