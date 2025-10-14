# GLTron Joystick/Gamepad Support

## Supported Controllers

GLTron now supports various game controllers including:
- Xbox Controllers (360, One, Series X/S)
- PlayStation Controllers (DualShock 4, DualSense)
- **Steam Deck Controller** (optimized)
- Generic USB Gamepads

## Control Mapping

### In-Game Controls
- **Left Analog Stick / D-Pad**: Turn left/right
- **L1/LB**: Quick turn left
- **R1/RB**: Quick turn right
- **Start**: Pause game
- **L3 (Left Stick Click)**: Change camera view
- **Right Analog Stick**: Rotate camera (in circle cam mode)

### Menu Controls
- **Left Analog Stick / D-Pad**: Navigate menu
- **A/Cross**: Select/Confirm
- **B/Circle**: Back/Cancel
- **Start**: Pause

### Pause Menu
- **A/Cross or Start**: Resume game
- **B/Circle**: Return to main menu

## Steam Deck Optimization

The game is specially optimized for Steam Deck with:
- Adjusted deadzone for Steam Deck's analog sticks (20%)
- Full button mapping for all Steam Deck controls
- Smooth analog stick turning
- Camera control with right stick

## Configuration

### Enable Joystick
1. Go to Menu → Misc Options
2. Select "Input Method"
3. Choose "Joystick"

### Adjust Settings
The joystick deadzone can be adjusted in the settings if needed.
Default is 15% (20% for Steam Deck).

## Troubleshooting

### Controller Not Detected
- Make sure your controller is connected before starting the game
- On Linux, ensure you have proper permissions for /dev/input/js*
- You may need to add your user to the 'input' group:
  ```bash
  sudo usermod -a -G input $USER
  ```
  Then log out and back in.

### Steam Deck Specific
- The Steam Deck controller should be automatically detected
- If using through Steam, ensure Steam Input is configured properly
- You can also use the touchpads for additional controls if configured in Steam

### Controller Lag
- Try adjusting the deadzone in settings
- Ensure no other programs are using the controller
- On Steam Deck, close unnecessary background applications

## Technical Details

The joystick support uses FreeGLUT's joystick API which provides:
- Up to 3 analog axes through the callback (X, Y, Z)
- Up to 32 digital buttons
- Polling at 20ms intervals (50Hz)

Note: Due to FreeGLUT limitations, only 3 axes are fully supported.
For full controller support including triggers and second analog stick,
consider using the Steam Input configuration on Steam Deck.
