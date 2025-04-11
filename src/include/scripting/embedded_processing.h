#ifndef EMBEDDED_PROCESSING_H
#define EMBEDDED_PROCESSING_H

/* Process embedded configuration script */
void process_embedded_config(const char* script, const char* name);

/* Process joystick configuration */
void process_embedded_joystick(void);

/* Process path configuration */
void process_embedded_path(void);

/* Process video configuration */
void process_embedded_video(void);

/* Process console configuration */
void process_embedded_console(void);

/* Process menu configuration */
void process_embedded_menu(void);

/* Process HUD configuration */
void process_embedded_hud(void);

/* Process gauge configuration */
void process_embedded_gauge(void);

/* Process config configuration - renamed to avoid duplicate definition */
void process_embedded_config_file(void);

/* Process save configuration */
void process_embedded_save(void);

/* Process artpack configuration */
void process_embedded_artpack(void);

/* Process game configuration */
void process_embedded_game(void);

/* Process main configuration */
void process_embedded_main(void);

#endif /* EMBEDDED_PROCESSING_H */
