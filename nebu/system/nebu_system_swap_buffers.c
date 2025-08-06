/* Implementation of nebu_System_SwapBuffers function */

#include "base/nebu_system.h"
#include "video/nebu_video_system.h"

/* Swap buffers function */
void nebu_System_SwapBuffers(void) {
    /* Call the video system's swap buffers function */
    nebu_Video_SwapBuffers();
}