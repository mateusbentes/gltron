// Simple menu drawing function
void drawMenu(void) {
    int i;
    int screenWidth, screenHeight;
    
    // Get screen dimensions
    nebu_Video_GetDimension(&screenWidth, &screenHeight);
    printf("[menu] Drawing simple menu: %dx%d\n", screenWidth, screenHeight);
    
    // Set viewport to full screen
    glViewport(0, 0, screenWidth, screenHeight);
    
    // Clear the screen with a bright color to test
    glClearColor(0.2f, 0.0f, 0.2f, 1.0f);  // Purple background for testing
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Set up orthographic projection for 2D drawing
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, screenWidth, 0, screenHeight, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Disable depth testing for 2D drawing
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    
    // Draw a simple test rectangle in the center
    glColor3f(1.0f, 1.0f, 1.0f);  // White
    glBegin(GL_QUADS);
    glVertex2f(screenWidth/2 - 100, screenHeight/2 - 50);
    glVertex2f(screenWidth/2 + 100, screenHeight/2 - 50);
    glVertex2f(screenWidth/2 + 100, screenHeight/2 + 50);
    glVertex2f(screenWidth/2 - 100, screenHeight/2 + 50);
    glEnd();
    
    // Draw GLTron title
    glColor3f(0.0f, 1.0f, 1.0f);  // Cyan
    glBegin(GL_QUADS);
    glVertex2f(screenWidth/2 - 150, screenHeight - 150);
    glVertex2f(screenWidth/2 + 150, screenHeight - 150);
    glVertex2f(screenWidth/2 + 150, screenHeight - 100);
    glVertex2f(screenWidth/2 - 150, screenHeight - 100);
    glEnd();
    
    // Draw menu options as simple rectangles
    for (i = 0; i < MENU_OPTION_COUNT; i++) {
        int y = screenHeight/2 - 150 + i * 40;
        
        // Highlight selected option
        if (i == gSelectedOption) {
            glColor3f(1.0f, 1.0f, 0.0f);  // Yellow for selected
        } else {
            glColor3f(0.7f, 0.7f, 0.7f);  // Gray for unselected
        }
        
        // Draw option rectangle
        glBegin(GL_QUADS);
        glVertex2f(screenWidth/2 - 120, y);
        glVertex2f(screenWidth/2 + 120, y);
        glVertex2f(screenWidth/2 + 120, y + 30);
        glVertex2f(screenWidth/2 - 120, y + 30);
        glEnd();
    }
    
    // Restore OpenGL state
    glEnable(GL_DEPTH_TEST);
    
    // Swap buffers
    nebu_System_SwapBuffers();
    printf("[menu] Simple menu drawn and buffers swapped\n");
}