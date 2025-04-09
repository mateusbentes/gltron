print("[lua] Loading basics.lua")

-- Try to load SDL2 compatibility layer
local success, err = pcall(function()
    dofile("scripts/sdl2_compat.lua")
end)

if not success then
    print("[lua] Error loading SDL2 compatibility layer: " .. tostring(err))
    -- Create a minimal SDL table if loading failed
    if not SDL then
        SDL = {
            KEYDOWN = 2,
            KEYUP = 3,
            MOUSEMOTION = 4,
            MOUSEBUTTONDOWN = 5,
            MOUSEBUTTONUP = 6,
            QUIT = 12,
            KMOD_NONE = 0,
            KMOD_SHIFT = 3,
            KMOD_CTRL = 192,
            KMOD_ALT = 768,
            BUTTON_LEFT = 1,
            BUTTON_MIDDLE = 2,
            BUTTON_RIGHT = 3
        }
    end
end

-- Define basic constants and functions
PI = 3.14159265358979323846

function script_print(...)
   io.write("[script] ")
   for i=1,arg.n do
      io.write(arg[i])
   end
   io.write("\n")
end

function clamp(value, min, max)
    if value < min then return min end
    if value > max then return max end
    return value
end

function lerp(a, b, t)
    return a + (b - a) * t
end

function smoothstep(a, b, t)
    local x = clamp((t - a) / (b - a), 0.0, 1.0)
    return x * x * (3 - 2 * x)
end

print("[lua] basics.lua loaded")
