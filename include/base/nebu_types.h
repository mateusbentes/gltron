#ifndef NEBU_TYPES_H
#define NEBU_TYPES_H

#include <math.h>

// Android stub for nebu_types.h

// Basic types
typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned int dword;

// Math constants
#ifndef PI
#define PI 3.14159265358979323846
#endif

// 2D vector
typedef struct {
    union {
        struct { float x, y; };
        float v[2];
    };
} vec2;

// 3D vector
typedef struct {
    union {
        struct { float x, y, z; };
        float v[3];
    };
} vec3;

// 2D bounding box
typedef struct {
    vec2 vMin, vMax;
} box2;

// 3D bounding box
typedef struct {
    vec3 vMin, vMax;
} box3;

// Rectangle
typedef struct {
    float x, y, w, h;
} nebu_Rect;

// System time
typedef struct {
    long tv_sec;
    long tv_usec;
    long current;
} SystemTime;

// Linked list
typedef struct nebu_List_s {
    struct nebu_List_s *next;
    struct nebu_List_s *prev;
    void *data;
} nebu_List;

// Segment (line)
typedef struct {
    vec3 vStart, vEnd;
} segment2;

// Function declarations
float segment2_Length(const segment2* s);

// Forward declarations for game types
typedef struct Player Player;
typedef struct Data Data;

// Constants
#define MAX_PLAYER_VISUALS 4

#endif /* NEBU_TYPES_H */

