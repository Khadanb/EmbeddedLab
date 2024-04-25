#include "game_struct.h"
#include "game_animation.h"
#include <stdlib.h>
#include <math.h>

enum contact hitbox_contact(const Entity *A, const Entity *B) {
	float leftA = A->position.x;
	float rightA = A->position.x + A->position.width;
	float topA = A->position.y;
	float bottomA = A->position.y + A->position.height;

	float leftB = B->position.x;
	float rightB = B->position.x + B->position.width;
	float topB = B->position.y;
	float bottomB = B->position.y + B->position.height;

	// Check for no collision
	if (rightA <= leftB || leftA >= rightB || topA >= bottomB || bottomA <= topB) {
		return NONE;
	}

	// Calculate penetration depths
	float leftPenetration = rightA - leftB;
	float rightPenetration = rightB - leftA;
	float topPenetration = bottomA - topB;
	float bottomPenetration = bottomB - topA;

	// Determine the minimal penetration direction
	float minPenetration = fmin(fmin(leftPenetration, rightPenetration), fmin(topPenetration, bottomPenetration));

	if (minPenetration == leftPenetration) {
		return RIGHT;
	} else if (minPenetration == rightPenetration) {
		return LEFT;
	} else if (minPenetration == topPenetration) {
		return DOWN;
	} else if (minPenetration == bottomPenetration) {
		return UP;
	}

	return NONE;
}


void new_game(Game *game) {
	game->camera_pos = 0;
	game->game_state = GAME_START;

	// Initialize all entities to inactive
	for (int i = 0; i < MAX_ENTITIES; i++) {
		game->entities[i].state.active = 0;
	}

	// Initialize Mario
	Entity *mario = &game->entities[0];
	*mario = (Entity){
		.position = {128, 128, 16, 16},
		.motion = {0, 0, 0, 0},
		.render = {ANI_MARIO_S_NORMAL, 1, 0},
		.state = {1, STATE_NORMAL, 0, TYPE_MARIO_SMALL}
	};


	// Initialize Ground Blocks
	int ground_positions[] = {GROUND_0_L, GROUND_1_L, GROUND_2_L, GROUND_3_L};
	int ground_widths[] = {GROUND_0_W, GROUND_1_W, GROUND_2_W, GROUND_3_W};
	for (int i = 0; i < GROUND_NUM; i++) {
		Entity *ground = &game->entities[i + 1]; // +1 to skip Mario
		*ground = (Entity){
			.position = {ground_positions[i], GROUND_LEVEL, ground_widths[i], 32},
			.motion = {0, 0, 0, 0},
			.render = {0, 1, 0},
			.state = {1, BLOCK_NORMAL, 0, TYPE_GROUND}
		};
	}

	// Initialize Blocks
	int block_start_index = GROUND_NUM + 1; // Start after Mario and Grounds
	for (int i = 0; i < BLOCK_NUM; i++) {
		Entity *block = &game->entities[block_start_index + i];
		*block = (Entity){
			.position = {320 + 16 * i, 304, 16, 16},
			.motion = {0, 0, 0, 0},
			.render = {ANI_BLOCK_A1, 1, 0},
			.state = {1, BLOCK_NORMAL, 0, TYPE_BLOCK_A}
		};
	}

	// Initialize Coins
	int coin_positions[][2] = {{324, 352}, {964, 352}, {324, 288}, {760, 352}, {808, 352}, {1228, 288}, {1244, 288}};
	int coin_index = block_start_index + BLOCK_NUM;
	for (int i = 0; i < COIN_NUM; i++) {
		Entity *coin = &game->entities[coin_index + i];
		*coin = (Entity){
			.position = {coin_positions[i % (sizeof(coin_positions) / sizeof(coin_positions[0]))][0], coin_positions[i % (sizeof(coin_positions) / sizeof(coin_positions[0]))][1], 8, 16},
			.motion = {0, 0, 0, 0},
			.render = {ANI_COIN_1, 1, 0},
			.state = {1, STATE_NORMAL, 0, TYPE_COIN}
		};
	}

	int mushroom_positions[][2] = {{150, 200}, {500, 300}, {800, 250}};
	int mushroom_index = coin_index + COIN_NUM;
	for (int i = 0; i < MUSH_NUM; i++) {
		Entity *mushroom = &game->entities[mushroom_index + i];
		*mushroom = (Entity){
			.position = {mushroom_positions[i % (sizeof(mushroom_positions) / sizeof(mushroom_positions[0]))][0], mushroom_positions[i % (sizeof(mushroom_positions) / sizeof(mushroom_positions[0]))][1], 16, 16},
			.motion = {0, 0, 0, 0},
			.render = {ANI_MUSH_NORMAL, 1, 0},
			.state = {0, STATE_NORMAL, 0, TYPE_MUSHROOM}
		};
	}

	// Initialize Goombas
	int goomba_positions[][2] = {{324, 268}, {820, 208}, {1224, 272}};
	int goomba_index = mushroom_index + MUSH_NUM; // Start after mushrooms
	for (int i = 0; i < GOOMBA_NUM; i++) {
		Entity *goomba = &game->entities[goomba_index + i];
		*goomba = (Entity){
			.position = {
				goomba_positions[i % (sizeof(goomba_positions) / sizeof(goomba_positions[0]))][0],
				goomba_positions[i % (sizeof(goomba_positions) / sizeof(goomba_positions[0]))][1],
				16, 16
			},
			.motion = {0, 0, 0, 0},
			.render = {ANI_GOOMBA_NORMAL, 1, 0},
			.state = {1, STATE_NORMAL, 0, TYPE_GOOMBA}
		};
	}

	// Initialize Tubes
	int tube_positions[][2] = {{416, 320}, {850, 320}};
	int tube_index = goomba_index + GOOMBA_NUM; // Start after Goombas
	for (int i = 0; i < TUBE_NUM; i++) {
		Entity *tube = &game->entities[tube_index + i];
		*tube = (Entity){
			.position = {
				tube_positions[i % (sizeof(tube_positions) / sizeof(tube_positions[0]))][0],
				tube_positions[i % (sizeof(tube_positions) / sizeof(tube_positions[0]))][1],
				32, 160
			},
			.motion = {0, 0, 0, 0},
			.render = {ANI_TUBE_H, 1, 0},
			.state = {1, STATE_NORMAL, 0, TYPE_TUBE}
		};
	}

	// Initialize Clouds
	int cloud_positions[][2] = {{240, 176}, {390, 144}};
	int cloud_index = tube_index + TUBE_NUM; // Start after Tubes
	for (int i = 0; i < CLOUD_NUM; i++) {
		Entity *cloud = &game->entities[cloud_index + i];
		*cloud = (Entity){
			.position = {
				cloud_positions[i % (sizeof(cloud_positions) / sizeof(cloud_positions[0]))][0],
				cloud_positions[i % (sizeof(cloud_positions) / sizeof(cloud_positions[0]))][1],
				64, 32
			},
			.motion = {0, 0, 0, 0},
			.render = {ANI_CLOUD_NORMAL, 1, 0},
			.state = {1, STATE_NORMAL, 0, TYPE_CLOUD}
		};
	}
}
