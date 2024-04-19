#ifndef _MARIO_GAME_ANIMATION
#define _MARIO_GAME_ANIMATION

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

// Mush Animation
#define ANI_MUSH_NORMAL 0
 
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
 
// Cloud Animation
#define ANI_CLOUD_NORMAL 0

// Coin Animation  
#define ANI_COIN_1 0
#define ANI_COIN_2 1
#define ANI_COIN_3 2
#define ANI_COIN_4 3

void mario_animation(mario_game *game_0, mario* mario_0, int f_counter);
void mush_animation(mario_game *game_0, mush* mush_0, int f_counter);
void goomba_animation(mario_game *game_0, goomba* goomba_0, int f_counter);
void tube_animation(mario_game *game_0, tube* tube_0, int f_counter);
void block_animation(mario_game *game_0, block* block_0, int f_counter);
void cloud_animation(mario_game *game_0, cloud* cloud_0, int f_counter);
void coin_animation(mario_game *game_0, coin* coin_0, int f_counter);
#endif
