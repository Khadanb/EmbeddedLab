#include "game_animation.h"
#include <stdio.h>

void animate_entity(Game *game, Entity *entity, int f_counter) {
	if (entity->state.active && entity->render.visible) {
		switch (entity->state.type) {
			case TYPE_MARIO_SMALL:
			case TYPE_MARIO_LARGE:
				animate_mario(game, entity, f_counter);
				break;
			case TYPE_GOOMBA:
				animate_goomba(game, entity, f_counter);
				break;
			case TYPE_BLOCK_A:
			case TYPE_BLOCK_B_1:
			case TYPE_BLOCK_B_2:
			case TYPE_BLOCK_B_3:
			case TYPE_BLOCK_B_4:
			case TYPE_BLOCK_B_16:
			case TYPE_BLOCK_A_H_8:
			case TYPE_BLOCK_OBJ_C:
			case TYPE_BLOCK_OBJ_M:
				animate_block(game, entity, f_counter);
				break;
			case TYPE_TUBE:
				animate_tube(game, entity, f_counter);
				break;
			case TYPE_GROUND:
				break;
			default:
				break;
		}
	}
}

void animate_mario(Game *game, Entity *entity, int f_counter) {
    if (entity->state.active && entity->render.visible) {
        int counter = f_counter % FRAME_LIMIT;  // Use the existing frame count within the animation cycle limit
        int rel_counter = counter - entity->state.animate_frame_counter;
        entity->state.animate_frame_counter = counter;  // Update the last animation frame counter

        switch (entity->state.state) {
            case STATE_NORMAL:
                // Determine the walking animation based on Mario's horizontal acceleration
                if (entity->motion.ax == 0) {
                    entity->render.pattern_code = ANI_MARIO_S_NORMAL;  // Standing still
                } else {
                    // If Mario is shutting or changing direction abruptly
                    if (entity->motion.ax * entity->motion.vx < 0) {
                        entity->render.pattern_code = ANI_MARIO_S_SHUT;
                    } else {
                        // Cycle through walking animations based on the frame count
                        int ani_div = (counter / 20) % 3;  // Use a slower frame update interval
                        entity->render.pattern_code = (ani_div == 0) ? ANI_MARIO_S_WALK1 :
                                                          (ani_div == 1) ? ANI_MARIO_S_WALK2 :
                                                          ANI_MARIO_S_WALK3;
                    }
                }
                break;

            case STATE_HIT:
                // Flash between hit animations for visual effect
                entity->render.pattern_code = (rel_counter / 3) % 2 ? ANI_MARIO_L_HIT : ANI_MARIO_S_HIT;
                break;

            case STATE_DEAD:
                // Dead animation should not adjust position, just set the appropriate sprite
                entity->render.pattern_code = ANI_MARIO_S_DEAD;
                break;

            case STATE_LARGE:
                // Determine the large Mario walking animation based on his horizontal acceleration
                if (entity->motion.ax == 0) {
                    entity->render.pattern_code = ANI_MARIO_L_NORMAL;  // Standing still
                } else {
                    if (entity->motion.ax * entity->motion.vx < 0) {
                        entity->render.pattern_code = ANI_MARIO_L_SHUT;
                    } else {
                        int ani_div = (counter / 20) % 3;  // Slower frame update interval
                        entity->render.pattern_code = (ani_div == 0) ? ANI_MARIO_L_WALK1 :
                                                          (ani_div == 1) ? ANI_MARIO_L_WALK2 :
                                                          ANI_MARIO_L_WALK3;
                    }
                }
                break;
        }

        // Only toggle visibility for non-normal states for blinking effect
        entity->render.visible = (entity->state.state == STATE_NORMAL || entity->state.state == STATE_LARGE) ? 1 : (counter / 30) % 2;
    }
}


void animate_goomba(Game *game, Entity *entity, int f_counter) {
	if (entity->state.active) {
		int frame_count = FRAME_LIMIT;

		int rel_counter = (f_counter - entity->state.animate_frame_counter + frame_count) % frame_count;

		if (entity->state.state == STATE_NORMAL) {
			entity->render.pattern_code = ANI_GOOMBA_NORMAL;

		} else if (entity->state.state == STATE_ANIMATE) {
			if (rel_counter == 0) {
				entity->position.y += 8;
			} else if (rel_counter >= 25) {
				entity->state.active = 0;
			}
			entity->render.pattern_code = ANI_GOOMBA_HIT;
		}

		entity->render.visible = 1;
	}
}

void animate_tube(Game *game, Entity *entity, int f_counter) {
	if (entity->state.active) {
		entity->render.visible = 1;
	}
}

void animate_block(Game *game, Entity *entity, int f_counter) {
	if (entity->state.active) {
		int frame_count = FRAME_LIMIT;
		int counter = (f_counter >= entity->state.animate_frame_counter) ?
					  f_counter : f_counter + frame_count;
		int rel_counter = counter - entity->state.animate_frame_counter;

		switch (entity->state.state) {
			case STATE_NORMAL:
				switch (entity->state.type) {
					case TYPE_BLOCK_A:
						entity->render.pattern_code = ANI_BLOCK_A1;
						break;
					case TYPE_BLOCK_B_1:
						entity->render.pattern_code = ANI_BLOCK_B;
						break;
					case TYPE_BLOCK_B_2:
						entity->render.pattern_code = ANI_BLOCK_B_V2;
						break;
					case TYPE_BLOCK_B_3:
						entity->render.pattern_code = ANI_BLOCK_B_V3;
						break;
					case TYPE_BLOCK_B_4:
						entity->render.pattern_code = ANI_BLOCK_B_V4;
						break;
					case TYPE_BLOCK_B_16:
						entity->render.pattern_code = ANI_BLOCK_B_16;
						break;
					case TYPE_BLOCK_A_H_8:
						entity->render.pattern_code = ANI_BLOCK_A_H8;
						break;
					case TYPE_BLOCK_OBJ_C:
					case TYPE_BLOCK_OBJ_M:
						entity->render.pattern_code = ANI_BLOCK_ITEM1;
						break;
					default:
						entity->render.pattern_code = ANI_BLOCK_ITEM_EMP;
						break;
				}
				break;
			case STATE_ANIMATE:

				if (rel_counter < 4) {
					entity->position.y -= 1;
					entity->render.pattern_code = ANI_BLOCK_ITEM_HIT;
				} else if (rel_counter < 8) {
					entity->position.y += 1;
					entity->render.pattern_code = ANI_BLOCK_ITEM_HIT;
				} else {
					entity->state.state = STATE_NORMAL;
				}
				break;
			default:
				entity->render.pattern_code = ANI_BLOCK_ITEM_EMP;
				break;
		}
	}
}
