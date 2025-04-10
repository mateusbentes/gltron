/* Platform-specific configuration for GLtron */
#ifndef GLTRON_CONFIG_H
#define GLTRON_CONFIG_H

/* Version */
#define VERSION "0.71"

/* Platform-specific directory paths */
#ifdef _WIN32
  /* Windows */
  #define PREF_DIR "~/AppData/Roaming/GLTron"
  #define SNAP_DIR "~/Pictures/GLTron"
  #define DATA_DIR "."  /* Current directory on Windows */
  #define RC_NAME "gltron.ini"
#elif defined(__APPLE__)
  /* macOS */
  #define PREF_DIR "~/Library/Preferences/GLTron"
  #define SNAP_DIR "~/Pictures/GLTron"
  #define DATA_DIR "."  /* Current directory on macOS */
  #define RC_NAME "com.gltron.plist"
#elif defined(__ANDROID__)
  /* Android */
  #define PREF_DIR "."  /* App's private storage */
  #define SNAP_DIR "."  /* App's private storage */
  #define DATA_DIR "."  /* App's assets directory */
  #define RC_NAME "gltron.cfg"
#else
  /* Linux/Unix - keep original paths */
  #define PREF_DIR "~/.gltronrc"
  #define SNAP_DIR "~/gltron-snapshots"
  #define DATA_DIR "/usr/local/share/gltron"
  #define RC_NAME ".gltronrc"
#endif

/* Platform-specific function replacements */
#ifdef __APPLE__
  /* Mac OS X doesn't have sinf and others in older versions */
  #define sinf(x) sin(x)
  #define cosf(x) cos(x)
  #define sqrtf(x) sqrt(x)
  #define tanf(x) tan(x)
  #define acosf(x) acos(x)
#endif

#endif /* GLTRON_CONFIG_H */
