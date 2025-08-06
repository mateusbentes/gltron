// Simple menu test to verify OpenGL rendering
#include <GL/gl.h>
#include <stdio.h>

void drawSimpleMenu(int screenWidth, int screenHeight) {
    printf("[menu] Drawing simple menu: %dx%d\n", screenWidth, screenHeight);
    
    // Set viewport
    glViewport(0, 0, screenWidth, screenHeight);
    
    // Clear screen with red background to test
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);  // Red background
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Set up 2D projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, screenWidth, 0, screenHeight, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Disable depth test for 2D
    glDisable(GL_DEPTH_TEST);
    
    // Draw a simple white rectangle in the center
    glColor3f(1.0f, 1.0f, 1.0f);  // White
    glBegin(GL_QUADS);
    glVertex2f(screenWidth/2 - 100, screenHeight/2 - 50);
    glVertex2f(screenWidth/2 + 100, screenHeight/2 - 50);
    glVertex2f(screenWidth/2 + 100, screenHeight/2 + 50);
    glVertex2f(screenWidth/2 - 100, screenHeight/2 + 50);
    glEnd();
    
    // Draw a blue rectangle above it
    glColor3f(0.0f, 0.0f, 1.0f);  // Blue
    glBegin(GL_QUADS);
    glVertex2f(screenWidth/2 - 50, screenHeight/2 + 100);
    glVertex2f(screenWidth/2 + 50, screenHeight/2 + 100);
    glVertex2f(screenWidth/2 + 50, screenHeight/2 + 150);
    glVertex2f(screenWidth/2 - 50, screenHeight/2 + 150);
    glEnd();
    
    printf("[menu] Simple menu drawn\n");
}