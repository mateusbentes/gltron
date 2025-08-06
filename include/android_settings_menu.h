#ifndef ANDROID_SETTINGS_MENU_H
#define ANDROID_SETTINGS_MENU_H

#ifdef __cplusplus
extern "C" {
#endif

// Menu visibility and navigation
void android_settings_menu_show(void);
void android_settings_menu_hide(void);
int android_settings_menu_is_visible(void);

// Menu navigation
void android_settings_menu_navigate_up(void);
void android_settings_menu_navigate_down(void);
void android_settings_menu_navigate_left(void);
void android_settings_menu_navigate_right(void);
void android_settings_menu_select(void);

// Menu information for rendering
const char* android_settings_menu_get_current_title(void);
int android_settings_menu_get_item_count(void);
int android_settings_menu_get_selected_item(void);
const char* android_settings_menu_get_item_name(int index);
const char* android_settings_menu_get_item_value(int index);
const char* android_settings_menu_get_item_description(int index);

// Special menu functions
void android_settings_menu_show_performance_info(void);

// Menu animation
void android_settings_menu_update(float delta_time);
float android_settings_menu_get_alpha(void);

#ifdef __cplusplus
}
#endif

#endif /* ANDROID_SETTINGS_MENU_H */

