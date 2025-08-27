package org.gltron.game;

import android.app.Activity;
import android.view.View;

public final class UiHelpers {
    private UiHelpers() {}

    private static final class SetUiVisibilityRunnable implements Runnable {
        private final View decorView;
        private final int flags;
        SetUiVisibilityRunnable(View decorView, int flags) {
            this.decorView = decorView;
            this.flags = flags;
        }
        @Override public void run() {
            if (decorView != null) {
                decorView.setSystemUiVisibility(flags);
            }
        }
    }

    public static void applyImmersive(final Activity activity, final View decorView, final int flags) {
        if (activity == null || decorView == null) return;
        try {
            activity.runOnUiThread(new SetUiVisibilityRunnable(decorView, flags));
        } catch (Throwable t) {
            // Best-effort: ignore if cannot post to UI thread
        }
    }
}
