-- SDL2 compatibility layer for Lua scripts
print("[lua] Loading SDL2 compatibility layer")

-- Create a table to store SDL 1.2 constants if they don't exist
if not SDL then
    SDL = {}
end

-- SDL 1.2 event types
SDL.KEYDOWN = 2
SDL.KEYUP = 3
SDL.MOUSEMOTION = 4
SDL.MOUSEBUTTONDOWN = 5
SDL.MOUSEBUTTONUP = 6
SDL.QUIT = 12

-- SDL 1.2 key modifiers
SDL.KMOD_NONE = 0x0000
SDL.KMOD_LSHIFT = 0x0001
SDL.KMOD_RSHIFT = 0x0002
SDL.KMOD_LCTRL = 0x0040
SDL.KMOD_RCTRL = 0x0080
SDL.KMOD_LALT = 0x0100
SDL.KMOD_RALT = 0x0200

-- Define bit operations if not available
if not bit_or then
    function bit_or(a, b)
        return a | b
    end
end

SDL.KMOD_SHIFT = bit_or(SDL.KMOD_LSHIFT, SDL.KMOD_RSHIFT)
SDL.KMOD_CTRL = bit_or(SDL.KMOD_LCTRL, SDL.KMOD_RCTRL)
SDL.KMOD_ALT = bit_or(SDL.KMOD_LALT, SDL.KMOD_RALT)

-- SDL 1.2 mouse buttons
SDL.BUTTON_LEFT = 1
SDL.BUTTON_MIDDLE = 2
SDL.BUTTON_RIGHT = 3

-- SDL 1.2 video flags
SDL.SWSURFACE = 0x00000000
SDL.HWSURFACE = 0x00000001
SDL.ASYNCBLIT = 0x00000004
SDL.ANYFORMAT = 0x10000000
SDL.HWPALETTE = 0x20000000
SDL.DOUBLEBUF = 0x40000000
SDL.FULLSCREEN = 0x80000000
SDL.OPENGL = 0x00000002
SDL.RESIZABLE = 0x00000010

print("[lua] SDL2 compatibility layer loaded")
