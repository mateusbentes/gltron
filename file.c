#include "gltron.h"
#include <string.h>
#include <errno.h>

#ifdef ANDROID
#include <android/asset_manager.h>  // For Android asset management
#include <android/asset_manager_jni.h>  // For AAssetManager functions
#include <sys/stat.h>  // For mkdir
#include <sys/types.h>  // For mkdir
#include <limits.h>
#ifndef PATH_MAX
#define PATH_MAX 1024
#endif
#endif

// Defensive, agnostic path resolution; never segfaults.
char* getFullPath(char *filename) {
  // Basic sanity
  if (filename == NULL) {
    return NULL;
  }
  if (filename[0] == '\0') {
    return NULL;
  }

#ifdef ANDROID
  // If an absolute path is provided, try to use it directly from filesystem.
  if (filename[0] == '/') {
    FILE* fp_abs = fopen(filename, "rb");
    if (fp_abs) {
      fclose(fp_abs);
      size_t len = strlen(filename);
      char* ret = (char*)malloc(len + 1);
      if (!ret) return NULL;
      memcpy(ret, filename, len + 1);
      return ret;
    }
    // If absolute path is invalid, fall through to asset lookup
  }
  // On Android, prefer APK assets using AAssetManager if available.
  extern char s_base_path[PATH_MAX]; // defined in android_glue.c, updated via gltron_set_base_path
  extern AAssetManager* g_android_asset_mgr; // defined in android_glue.c

  if (g_android_asset_mgr) {
    // Debug log
    __android_log_print(ANDROID_LOG_INFO, "GLTron", "getFullPath: trying asset '%s'", filename ? filename : "(null)");
    AAsset* asset = AAssetManager_open(g_android_asset_mgr, filename, AASSET_MODE_STREAMING);
    if (asset) {
      __android_log_print(ANDROID_LOG_INFO, "GLTron", "getFullPath: asset open OK '%s'", filename);
      // Compose destination path under base path
      char tmpPath[PATH_MAX];
      tmpPath[0] = '\0';
      // Ensure s_base_path is NUL-terminated and non-empty
      size_t base_len = strnlen(s_base_path, sizeof(s_base_path));
      if (base_len == 0 || base_len >= sizeof(s_base_path)) {
        // Cannot safely extract; close and return NULL
        AAsset_close(asset);
        return NULL;
      }

      // Ensure base directory exists (best-effort); ignore errors
      mkdir(s_base_path, 0700);

      // Join base + '/' + filename; guard against overflow
      int n = snprintf(tmpPath, sizeof(tmpPath), "%s/%s", s_base_path, filename);
      if (n < 0 || (size_t)n >= sizeof(tmpPath)) {
        AAsset_close(asset);
        return NULL;
      }

      // Ensure intermediate directories exist for nested assets
      // Walk through tmpPath and create directories for each '/'
      for (char* p = tmpPath + base_len + 1; *p; ++p) {
        if (*p == '/') {
          *p = '\0';
          mkdir(tmpPath, 0700);
          *p = '/';
        }
      }

      // If target exists, skip re-extract (simple cache)
      FILE* existed = fopen(tmpPath, "rb");
      if (existed) {
        fclose(existed);
        size_t len = strlen(tmpPath);
        char* ret = (char*)malloc(len + 1);
        if (!ret) { AAsset_close(asset); return NULL; }
        memcpy(ret, tmpPath, len + 1);
        __android_log_print(ANDROID_LOG_INFO, "GLTron", "getFullPath: using cached '%s'", tmpPath);
        AAsset_close(asset);
        return ret;
      }

      // Create the file and write the asset data to it
      FILE* out = fopen(tmpPath, "wb");
      if (out) {
        const size_t bufSize = 4096;
        char buf[bufSize];
        int nread;
        int ok = 1;
        while ((nread = AAsset_read(asset, buf, bufSize)) > 0) {
          size_t w = fwrite(buf, 1, (size_t)nread, out);
          if (w != (size_t)nread) { ok = 0; break; }
        }
        if (fclose(out) != 0) ok = 0;
        AAsset_close(asset);
        if (!ok) {
          __android_log_print(ANDROID_LOG_ERROR, "GLTron", "getFullPath: write failed '%s'", tmpPath);
          remove(tmpPath);
          return NULL;
        }
        size_t len = strlen(tmpPath);
        char* ret = (char*)malloc(len + 1);
        if (!ret) return NULL;
        memcpy(ret, tmpPath, len + 1);
        return ret;
      }

      // Could not open output file; return NULL instead of inconsistent fallback
      __android_log_print(ANDROID_LOG_ERROR, "GLTron", "getFullPath: couldn't open output '%s'", tmpPath);
      AAsset_close(asset);
      return NULL;
    } else {
      __android_log_print(ANDROID_LOG_ERROR, "GLTron", "getFullPath: AAssetManager_open failed for '%s'", filename);
    }
  }
#endif

#ifdef ANDROID
  // If on Android and asset load fails, do not fall back to desktop paths
  return NULL;
#endif

  // For non-Android platforms
  char *path = NULL;
  FILE *fp = NULL;
  const char *share1 = "/usr/share/games/gltron";
  const char *share2 = "/usr/local/share/games/gltron";

  // 1) Try current directory ./filename (test readability first)
  fp = fopen(filename, "rb");
  if (fp) {
    fclose(fp);
    size_t len = strlen(filename);
    path = (char*)malloc(len + 1);
    if (!path) return NULL;
    memcpy(path, filename, len + 1);
    return path;
  }

  // 2) Try $GLTRON_HOME/filename
  const char *base = getenv("GLTRON_HOME");
  if (base && base[0]) {
    size_t base_len = strlen(base);
    size_t namelen = strlen(filename);
    size_t needed = base_len + 1 + namelen + 1; // base + sep + name + NUL
    path = (char*)malloc(needed);
    if (!path) return NULL;
    int n = snprintf(path, needed, "%s%c%s", base, SEPERATOR, filename);
    if (n >= 0 && (size_t)n < needed) {
      fp = fopen(path, "rb");
      if (fp) { fclose(fp); return path; }
    }
    free(path); path = NULL;
  }

  // 3) Try /usr/share/games/gltron/filename
  {
    size_t s1_len = strlen(share1);
    size_t namelen = strlen(filename);
    size_t needed = s1_len + 1 + namelen + 1;
    path = (char*)malloc(needed);
    if (!path) return NULL;
    int n = snprintf(path, needed, "%s%c%s", share1, SEPERATOR, filename);
    if (n >= 0 && (size_t)n < needed) {
      fp = fopen(path, "rb");
      if (fp) { fclose(fp); return path; }
    }
    free(path); path = NULL;
  }

  // 4) Try /usr/local/share/games/gltron/filename
  {
    size_t s2_len = strlen(share2);
    size_t namelen = strlen(filename);
    size_t needed = s2_len + 1 + namelen + 1;
    path = (char*)malloc(needed);
    if (!path) return NULL;
    int n = snprintf(path, needed, "%s%c%s", share2, SEPERATOR, filename);
    if (n >= 0 && (size_t)n < needed) {
      fp = fopen(path, "rb");
      if (fp) { fclose(fp); return path; }
    }
    free(path); path = NULL;
  }

  // Not found
  return NULL;
}
