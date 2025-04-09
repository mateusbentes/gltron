#ifndef _NEBU_GEOMETRY_H
#define _NEBU_GEOMETRY_H

#include "nebu_mesh.h"

/* Vector operations used by light.c */
void vsub4(const float v1[4], const float v2[4], float result[4]);
float scalarprod4(const float v1[4], const float v2[4]);
float length4(const float v[4]);

/* Geometry creation functions */
nebu_Mesh* nebu_geom_CreateCylinder(float radius, float radius2, float height, int slices, int stacks);

#endif
