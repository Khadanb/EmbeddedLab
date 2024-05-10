#ifndef _MARIO_GAME_STRUCT
#define _MARIO_GAME_STRUCT

#include <stdint.h>

#define MAX_ENTITIES 128

#define GRAVITY 0.23f
#define MAX_SPEED_H 1.85f
#define MAX_SPEED_V 4.6f
#define LOAD_LIMIT (5*16)
#define CAMERA_SIZE 1120
#define GROUND_LEVEL 368

#define HOLE_0_R 527
#define HOLE_1_L 1504
#define HOLE_2_R 2175

#define WALK_ACC (0.09)
#define SHUT_ACC (0.12)
#define JUMP_INIT_V_SMALL (4.6)
#define JUMP_INIT_V_LARGE (5.6)
#define MAX_SPEED_V_JUMP (8.1)
#define FRICTION (0.08)

#define BLOCK_VIS_LIMIT 9
#define COIN_VIS_LIMIT 4
#define GOOMBA_VIS_LIMIT 2
#define MUSH_VIS_LIMIT 1
#define TUBE_VIS_LIMIT 1
#define CLOUD_VIS_LIMIT 1

enum EntityState {
	STATE_NORMAL,
	STATE_ANIMATE,
	STATE_HIT,
	STATE_DEAD,
	STATE_LARGE,
	BLOCK_NORMAL,
	BLOCK_ANIMATE
};

enum contact {
	NONE, LEFT, RIGHT, UP, DOWN
};

enum EntityType {
	TYPE_MARIO_SMALL,
	TYPE_MARIO_LARGE,
	TYPE_GOOMBA,
	TYPE_BLOCK_A,
	TYPE_BLOCK_B_1,
	TYPE_BLOCK_B_2,
	TYPE_BLOCK_B_3,
	TYPE_BLOCK_B_4,
	TYPE_BLOCK_B_16,
	TYPE_BLOCK_A_H_8,
	TYPE_BLOCK_OBJ_C,
	TYPE_BLOCK_OBJ_M,
	TYPE_TUBE,
	TYPE_GROUND,
	TYPE_EMP
};

typedef struct {
	float x, y;
	int width, height;
} PositionComponent;

typedef struct {
	float vx, vy;
	float ax, ay;
} MotionComponent;

typedef struct {
	uint32_t pattern_code;
	int visible;
	int flip;
} RenderComponent;

typedef struct {
	int active;
	int state;
	int animate_frame_counter;
	int type;
} StateComponent;

typedef struct {
	PositionComponent position;
	MotionComponent motion;
	RenderComponent render;
	StateComponent state;
} Entity;


typedef struct {
	int camera_pos;
	float camera_velocity;
	int game_state;
	Entity entities[MAX_ENTITIES];
} Game;

enum {
	GAME_START,
	GAME_NORMAL,
	GAME_END
};

void new_game(Game *game);
enum contact hitbox_contact(const Entity *A, const Entity *B);

#endif
