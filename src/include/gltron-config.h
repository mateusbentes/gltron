/* Platform-specific configuration for GLtron */
#ifndef GLTRON_CONFIG_H
#define GLTRON_CONFIG_H

/* Directory paths */
#define PREF_DIR "~/.gltronrc"
#define SNAP_DIR "~/gltron-snapshots"
#define DATA_DIR "/usr/local/share/gltron"
#define VERSION "0.7.1"

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
