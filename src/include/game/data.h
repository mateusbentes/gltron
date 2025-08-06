#ifndef DATA_H
#define DATA_H

/* 
 * Include the header files that contain the original definitions
 * of vec2 and segment2 to avoid conflicts
 */
#include "base/nebu_vector.h"

/* 
 * The vec2 type is already defined in nebu_vector.h as:
 * typedef struct { float v[2]; } vec2;
 * So we don't need to redefine it here.
 */

/* 
 * EGameType enum is already defined elsewhere, so we'll comment it out here
 * to avoid redeclaration errors.
 */
#if 0
typedef enum EGameType {
  GAME_SINGLE = 1,
#ifdef RECORD
  GAME_SINGLE_RECORD = 2,
  GAME_PLAY = 4,
  GAME_PLAY_NETWORK = 8,
  GAME_NETWORK_RECORD
#endif
} EGameType;
#endif

#endif
