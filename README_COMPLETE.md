# GLTron - Complete Edition

## Features

### 🎮 Core Gameplay
- Classic light cycle battles from TRON
- Up to 4 players in split-screen or online
- Multiple camera modes
- Smooth 3D graphics with OpenGL

### 🔊 Sound System
- Full sound effects for all game events
- Background music support
- Menu navigation sounds
- Crash, win, and lose effects

### 🕹️ Input Support
- **Keyboard**: Classic WASD/Arrow controls
- **Mouse**: Point and click interface
- **Joystick/Gamepad**: Full controller support
- **Steam Deck**: Optimized controls and layout

### 🌐 Steam Multiplayer
- **Quick Match**: Instant matchmaking
- **Private Lobbies**: Create and join custom games
- **Spectator Mode**: Watch other players after elimination
- **P2P Networking**: Low latency gameplay
- **Auto-fill with AI**: Games start with AI, replaced by joining players

### 👁️ Spectator Mode
- Automatically enabled when you crash in multiplayer
- Switch between living players (non-AI preferred)
- Left/Right arrows to change spectated player
- Shows player type (Human/Remote/AI)

## Building

### Linux
```bash
# With all features
./build_with_steam.sh

# Without Steam
mkdir build && cd build
cmake .. -DUSE_SOUND=ON
make -j4
```

### Windows
```powershell
# Run PowerShell script
./build_windows_steam.ps1

# Or batch file
build_windows_steam.bat
```

### macOS
```bash
mkdir build && cd build
cmake .. -DUSE_STEAMWORKS=ON
make -j4
```

## Multiplayer Guide

### Quick Start
1. Launch GLTron with Steam running
2. Select **Multiplayer → Quick Match**
3. Game automatically:
   - Searches for existing lobbies
   - Joins if found, or creates new lobby
   - Starts with AI players
   - Replaces AI as players join

### Creating a Game
1. **Multiplayer → Create Private Lobby**
2. Game starts immediately with AI
3. Share lobby with friends
4. AI players are replaced as friends join

### Joining a Game
1. **Multiplayer → Join Lobby**
2. Browse available games
3. Select and join
4. You'll replace an AI player

### Spectator Mode
- Activates automatically when you crash
- Watch remaining human players
- Use Left/Right arrows to switch view
- Prioritizes human players over AI

## Controls

### Game Controls
| Input | Keyboard | Mouse | Gamepad | Steam Deck |
|-------|----------|-------|---------|------------|
| Turn Left | A/← | Swipe Left | L-Stick Left | L-Stick/D-Pad Left |
| Turn Right | D/→ | Swipe Right | L-Stick Right | L-Stick/D-Pad Right |
| Pause | ESC | - | Start | Start |
| Camera | F1-F3 | - | L3 | L3 |

### Spectator Controls
- **Left/Right Arrows**: Switch between players
- **ESC**: Return to menu

### Menu Navigation
- **Arrow Keys/D-Pad**: Navigate
- **Enter/A Button**: Select
- **ESC/B Button**: Back

## Network Architecture

### Peer-to-Peer System
- No dedicated server required
- Host manages game state
- Direct connections via Steam
- Automatic NAT traversal

### Synchronization
- Position updates: 20Hz (unreliable)
- Turn events: Immediate (reliable)
- Crash events: Immediate (reliable)
- Game state: Host authoritative

### Player Management
- Always 4 player slots
- Dynamic AI ↔ Human switching
- Seamless join/leave
- Host migration on disconnect

## Troubleshooting

### Steam Issues
- Ensure Steam is running
- Check steam_appid.txt (should contain "480")
- Verify Steamworks library is linked

### Multiplayer Issues
- Check firewall settings
- Ensure Steam overlay works
- Try creating lobby instead of joining

### Performance
- Reduce graphics settings
- Close background applications
- Check network latency

## Development

### Adding Features
The codebase is modular:
- `steam_multiplayer.*`: Core Steam integration
- `spectator.c`: Spectator mode logic
- `multiplayer_menu.c`: UI integration
- `lobby_browser.c`: Lobby browsing

### Testing
Using App ID 480 (Spacewar) for development.
For production, register your own App ID with Steam.

## Credits

- Original GLTron by Andreas Umbach
- Steam Integration & Multiplayer
- Spectator Mode Implementation
- Sound System Enhancement
- Joystick/Steam Deck Support

## License

GLTron is released under GPL v2.
Steam integration uses Steamworks SDK (see Steam license).

---

**Enjoy the game! Race, crash, and spectate in the Grid!**
