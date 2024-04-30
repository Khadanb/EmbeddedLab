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

    // Initialize Goomba
    Entity *goomba = &game->entities[1];
    *goomba = (Entity){
        .position = {300, 300, 16, 16},  
        .motion = {0, 0, 0, 0},
        .render = {ANI_GOOMBA_NORMAL, 1, 0},
        .state = {1, STATE_NORMAL, 0, TYPE_GOOMBA}
    };

    // Initialize Mushroom
    Entity *mushroom = &game->entities[2]; 
    *mushroom = (Entity){
        .position = {200, 300, 16, 16},
        .motion = {0, 0, 0, 0},
        .render = {ANI_MUSH_NORMAL, 1, 0},
        .state = {1,STATE_NORMAL, 0, TYPE_MUSHROOM}
    };

    // Initialize Ground
    Entity *ground = &game->entities[3];
    *ground = (Entity){
        .position = {0, GROUND_LEVEL, 640, 32},  
        .motion = {0, 0, 0, 0},
        .render = {0, 1, 0},
        .state = {1, STATE_NORMAL, 0, TYPE_GROUND}
    };

    Entity *ceiling = &game->entities[4];
    *ceiling = (Entity){
        .position = {0, 100, 640, 32},  
        .motion = {0, 0, 0, 0},
        .render = {0, 1, 0},
        .state = {1, STATE_NORMAL, 0, TYPE_GROUND}
    };
}