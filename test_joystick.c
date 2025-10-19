/* Simple joystick test program for GLTron */
#include <stdio.h>
#include <GL/freeglut.h>

void joystick_callback(unsigned int buttonMask, int x, int y, int z) {
    static int update_count = 0;
    static unsigned int last_buttons = 0;
    static int last_x = 0, last_y = 0, last_z = 0;
    
    update_count++;
    
    /* Only print when something changes */
    if (buttonMask != last_buttons || x != last_x || y != last_y || z != last_z) {
        printf("[%d] Buttons: 0x%04X, X: %d, Y: %d, Z: %d\n", 
               update_count, buttonMask, x, y, z);
        
        /* Decode buttons */
        for (int i = 0; i < 16; i++) {
            if ((buttonMask & (1 << i)) && !(last_buttons & (1 << i))) {
                printf("  Button %d pressed\n", i);
            } else if (!(buttonMask & (1 << i)) && (last_buttons & (1 << i))) {
                printf("  Button %d released\n", i);
            }
        }
        
        /* Decode stick movement */
        if (x < -500 && last_x >= -500) {
            printf("  Stick LEFT\n");
        } else if (x > 500 && last_x <= 500) {
            printf("  Stick RIGHT\n");
        }
        
        if (y < -500 && last_y >= -500) {
            printf("  Stick UP\n");
        } else if (y > 500 && last_y <= 500) {
            printf("  Stick DOWN\n");
        }
        
        last_buttons = buttonMask;
        last_x = x;
        last_y = y;
        last_z = z;
    }
}

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
    if (key == 27) {  /* ESC */
        exit(0);
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(400, 300);
    glutCreateWindow("Joystick Test");
    
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    
    /* Check for joystick */
    if (glutDeviceGet(GLUT_HAS_JOYSTICK)) {
        int buttons = glutJoystickGetNumButtons(0);
        int axes = glutJoystickGetNumAxes(0);
        printf("Joystick detected: %d buttons, %d axes\n", buttons, axes);
        
        /* Enable joystick polling */
        glutJoystickFunc(joystick_callback, 50);
        printf("Joystick polling enabled at 20Hz\n");
        printf("Press ESC to exit\n\n");
    } else {
        printf("No joystick detected\n");
        printf("Press ESC to exit\n");
    }
    
    glutMainLoop();
    return 0;
}

/* Compile with: gcc test_joystick.c -o test_joystick -lGL -lglut -lm */
