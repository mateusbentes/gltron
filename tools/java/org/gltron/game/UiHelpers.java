package org.gltron.game;

import android.app.Activity;
import android.os.Build;
import android.view.View;

/**
 * Minimal helper for applying immersive fullscreen flags from native code.
 * Compatible with API 19+ and resilient to missing features.
 */
public final class UiHelpers {
    private UiHelpers() {}

    public static void applyImmersive(final Activity activity, final View decor, final int flags) {
        if (activity == null || decor == null) return;
        try {
            activity.runOnUiThread(new Runnable() {
                @Override public void run() {
                    try {
                        int f = flags;
                        // Some devices need the low-profile flag to keep nav dimmed
                        if (Build.VERSION.SDK_INT >= 19) {
                            // ensure layout stable for consistent content area
                            f |= View.SYSTEM_UI_FLAG_LAYOUT_STABLE;
                        }
                        decor.setSystemUiVisibility(f);
                    } catch (Throwable t) {
                        // ignore; immersive is best-effort
                    }
                }
            });
        } catch (Throwable t) {
            // ignore; best-effort only
        }
    }
}
