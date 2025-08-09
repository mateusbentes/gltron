#include "video/nebu_renderer_gl.h"
#include "base/nebu_surface.h"
#include "video/nebu_video_system.h"
#include "video/nebu_video_utility.h"

// Helper: column-major ortho matrix (OpenGL style)
static void make_ortho_matrix(float *out, float left, float right, float bottom, float top, float near, float far) {
    memset(out, 0, sizeof(float) * 16);
    out[0] = 2.0f / (right - left);
    out[5] = 2.0f / (top - bottom);
    out[10] = -2.0f / (far - near);
    out[12] = -(right + left) / (right - left);
    out[13] = -(top + bottom) / (top - bottom);
    out[14] = -(far + near) / (far - near);
    out[15] = 1.0f;
}

void nebu_Video_rasonly(int x, int y, int width, int height, GLuint shaderProgram, GLint uMVP) {
    // Compute orthographic projection matrix
    float ortho[16];
    make_ortho_matrix(ortho, 0.0f, (float)width, 0.0f, (float)height, 0.0f, 1.0f);

    // Set the MVP matrix uniform in your shader
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(uMVP, 1, GL_FALSE, ortho);

    // Set the viewport
    glViewport(x, y, width, height);

    // No glMatrixMode, glLoadIdentity, glOrtho in OpenGL ES 2.0+
    // Model matrix should be identity (or set as needed in your shader)
}

nebu_Surface* nebu_Video_GrabScreen(int width, int height) { 
  nebu_Surface *pSurface = nebu_Surface_Create(width, height, NEBU_SURFACE_FMT_RGB);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pSurface->data);
  return pSurface;
}
