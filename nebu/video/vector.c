#include "video/nebu_geometry.h"
#include <math.h>

/* Vector subtraction: result = v1 - v2 */
void vsub4(const float v1[4], const float v2[4], float result[4]) {
    result[0] = v1[0] - v2[0];
    result[1] = v1[1] - v2[1];
    result[2] = v1[2] - v2[2];
    result[3] = v1[3] - v2[3];
}

/* Scalar product (dot product) of two vectors */
float scalarprod4(const float v1[4], const float v2[4]) {
    return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2] + v1[3] * v2[3];
}

/* Length (magnitude) of a vector */
float length4(const float v[4]) {
    return sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2] + v[3] * v[3]);
}