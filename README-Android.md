Android (NDK) integration guide

Overview
- This project includes Android-native hooks and a minimal NativeActivity entry point to run GLTron on Android using OpenGL ES 2.0 and EGL.
- Android-specific code is compiled only when ANDROID is set in the CMake toolchain (i.e., when building with the Android NDK).

Components
- android_main.c: Implements android_main() using android_native_app_glue, creates EGL context/surface, handles input, drives the render loop.
- android_glue.h/.c: Exposes C functions for initialization, resize, frame update, input, asset manager and base path configuration:
  - void gltron_init(void);
  - void gltron_resize(int width, int height);
  - void gltron_frame(void);
  - void gltron_on_key(int key, int action);
  - void gltron_on_touch(float x, float y, int action);
  - void gltron_set_asset_manager(void* asset_mgr);
  - void gltron_set_base_path(const char* base_path);
- file.c: When ANDROID is defined, attempts to load resources from APK assets via AAssetManager and extracts to a writable base path.

Building
1) Create a build directory for Android:
   - mkdir build-android && cd build-android
2) Configure with the Android NDK toolchain:
   - cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-21 ..
3) Build:
   - cmake --build . --config Release

Packaging and running
- Use NativeActivity in your AndroidManifest.xml:
  <application>
    <activity android:name="android.app.NativeActivity"
              android:label="gltron"
              android:configChanges="orientation|keyboardHidden|screenSize">
      <meta-data android:name="android.app.lib_name" android:value="gltron"/>
    </activity>
  </application>
- Place resource files in app/src/main/assets/ (e.g., settings.txt, menu.txt, textures, sounds).
- The engine will extract requested assets to the app's internal files directory (provided by NativeActivity->internalDataPath) and use filesystem paths expected by existing loaders.

Input mapping
- DPAD arrow keys map to GLUT-like special keys (100–103) for compatibility.
- Touch events are forwarded to GUI handlers (basic mapping provided; adjust as needed).

Notes
- By default, assets are extracted on demand to <internalDataPath>/<filename>. The code assumes the base path exists; ensure it is created (see below).
- Desktop GLUT calls are compiled out on Android. Rendering is driven by android_main.c.

Extending
- You can provide your own Java/Kotlin UI and call the C hooks via JNI instead of using NativeActivity.
- For advanced asset handling, consider memory-based loading instead of extraction.
