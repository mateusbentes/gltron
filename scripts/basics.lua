-- Load SDL2 compatibility layer first
dofile("scripts/sdl2_compat.lua")

print("[lua] Loading basics.lua")

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
