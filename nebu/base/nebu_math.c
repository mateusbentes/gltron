#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

float nebu_deg2rad(float deg) { return deg * ((float)M_PI / 180.0f); }
float nebu_tanf(float x) { return tanf(x); }
float nebu_sqrtf(float x) { return sqrtf(x); }