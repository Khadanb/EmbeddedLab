#ifndef _GAME_ANIMATION_H
#define _GAME_ANIMATION_H

#include "game_struct.h"

// Frame Limit
#define FRAME_LIMIT 6000

// Mario Animation
#define ANI_MARIO_S_NORMAL 0
#define ANI_MARIO_S_WALK1 1
#define ANI_MARIO_S_WALK2 2
#define ANI_MARIO_S_WALK3 3
#define ANI_MARIO_S_SHUT 4
#define ANI_MARIO_S_JUMP 5
#define ANI_MARIO_S_DEAD 6
#define ANI_MARIO_L_NORMAL 7
#define ANI_MARIO_L_WALK1 8
#define ANI_MARIO_L_WALK2 9
#define ANI_MARIO_L_WALK3 10
#define ANI_MARIO_L_SHUT 11
#define ANI_MARIO_L_JUMP 12
#define ANI_MARIO_L_SIT 13
#define ANI_MARIO_M_NORMAL 14
#define ANI_MARIO_L_HIT 15
#define ANI_MARIO_S_HIT 16
#define ANI_MARIO_S_HANG 17
#define ANI_MARIO_L_HANG 18

// Goomba Animation
#define ANI_GOOMBA_NORMAL 0
#define ANI_GOOMBA_HIT 1

// Tube Animation
#define ANI_TUBE_H 0
#define ANI_TUBE_B 1

// Block Animation
#define ANI_BLOCK_ITEM1 0
#define ANI_BLOCK_ITEM2 1
#define ANI_BLOCK_ITEM3 2
#define ANI_BLOCK_ITEM_HIT 3
#define ANI_BLOCK_ITEM_EMP 4
#define ANI_BLOCK_A1 5
#define ANI_BLOCK_A2 6
#define ANI_BLOCK_A_HIT 7
#define ANI_BLOCK_B 8
#define ANI_BLOCK_A_H2 9
#define ANI_BLOCK_A_H3 10
#define ANI_BLOCK_A_H8 11
#define ANI_BLOCK_B_V2 12
#define ANI_BLOCK_B_V3 13
#define ANI_BLOCK_B_V4 14
#define ANI_BLOCK_B_16 15

// Function pointer type for entity animation functions
typedef void (*AnimateFunc)(Game *game, Entity *entity, int f_counter);

// Animation function declarations
void animate_mario(Game *game, Entity *entity, int f_counter);
void animate_goomba(Game *game, Entity *entity, int f_counter);
void animate_block(Game *game, Entity *entity, int f_counter);
void animate_tube(Game *game, Entity *entity, int f_counter);

// Generic function to animate any entity
void animate_entity(Game *game, Entity *entity, int f_counter);

#endif // _GAME_ANIMATION_H
