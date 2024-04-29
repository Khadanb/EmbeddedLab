#include "game_animation.h"

void animate_entity(Game *game, Entity *entity, int f_counter) {
	if (entity->state.active && entity->render.visible) {
		switch (entity->state.type) {
			case TYPE_MARIO_SMALL:
			case TYPE_MARIO_LARGE:
				animate_mario(game, entity, f_counter);
				break;
			case TYPE_MUSHROOM:
				animate_mushroom(game, entity, f_counter);
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
			case TYPE_COIN:
				animate_coin(game, entity, f_counter);
				break;
			case TYPE_TUBE:
				animate_tube(game, entity, f_counter);
				break;
			case TYPE_CLOUD:
				animate_cloud(game, entity, f_counter);
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
		int frame_count = FRAME_LIMIT;
		int counter = f_counter % frame_count;
		int rel_counter = counter - entity->state.animate_frame_counter;
		entity->state.animate_frame_counter = counter;

		float dead_v = -4.7f;
		float dead_acc = 0.16f;

		switch (entity->state.state) {
			case STATE_NORMAL:
				if (entity->motion.ax == 0) {
					entity->render.pattern_code = ANI_MARIO_S_NORMAL;
				} else if (entity->motion.ax * entity->motion.vx < 0) {
					entity->render.pattern_code = ANI_MARIO_S_SHUT;
				} else {
					int ani_div = (counter / 6) % 3;
					entity->render.pattern_code = (ani_div == 0) ? ANI_MARIO_S_WALK1 :
												  (ani_div == 1) ? ANI_MARIO_S_WALK2 :
												  ANI_MARIO_S_WALK3;
				}
				break;
			case STATE_HIT:
				entity->render.pattern_code = (rel_counter / 3) % 2 ? ANI_MARIO_L_HIT : ANI_MARIO_S_HIT;
				if (rel_counter == 0) entity->position.y += 3;
				if (rel_counter > 20) entity->state.state = STATE_NORMAL;
				break;
			case STATE_ENLARGE:
				entity->render.pattern_code = (rel_counter / 3) % 2 ? ANI_MARIO_M_NORMAL : ANI_MARIO_L_NORMAL;
				if (rel_counter == 0) entity->position.y -= 20;
				if (rel_counter > 30) entity->state.state = STATE_NORMAL;
				break;
			case STATE_DEAD:
				entity->render.pattern_code = ANI_MARIO_S_DEAD;
				if (rel_counter == 0) entity->motion.vy = dead_v;
				else if (rel_counter > 30) {
					entity->position.y += entity->motion.vy;
					entity->motion.vy += dead_acc;
					if (entity->position.y > 1.01*GROUND_LEVEL) entity->state.state = STATE_DEAD;
				}
				break;
		}

		entity->render.visible = entity->state.state == STATE_NORMAL ? 1 : (counter / 8) % 2;
		entity->position.x = entity->position.x - game->camera_pos;
		entity->position.y = entity->position.y;
	}
}

void animate_mushroom(Game *game, Entity *entity, int f_counter) {
	if (entity->state.active) {
		int frame_count = FRAME_LIMIT;

		int rel_counter = (f_counter - entity->state.animate_frame_counter + frame_count) % frame_count;

		if (entity->state.state == STATE_NORMAL) {
			entity->render.pattern_code = ANI_MUSH_NORMAL;
		} else if (entity->state.state == STATE_ANIMATE) {
			entity->render.pattern_code = ANI_MUSH_NORMAL;
			if (rel_counter < 40) {
				entity->position.y -= 0.5;
			} else {
				entity->state.state = STATE_NORMAL;
			}
		}

		entity->render.visible = 1;
		entity->position.x = entity->position.x - game->camera_pos;
		entity->position.y = entity->position.y;
	}
}

void animate_goomba(Game *game, Entity *entity, int f_counter) {
	if (entity->state.active) {
		int frame_count = FRAME_LIMIT;

		int rel_counter = (f_counter - entity->state.animate_frame_counter + frame_count) % frame_count;

		if (entity->state.state == STATE_NORMAL) {
			// int ani_div = (f_counter / 7) % 2;
			entity->render.pattern_code = ANI_GOOMBA_NORMAL;
			// entity->render.flip = ani_div;
		} else if (entity->state.state == STATE_ANIMATE) {
			if (rel_counter == 0) {
				entity->position.y += 8;
			} else if (rel_counter >= 25) {
				entity->state.active = 0;
			}
			entity->render.pattern_code = ANI_GOOMBA_HIT;
		}

		entity->render.visible = 1;
		entity->position.x = entity->position.x - game->camera_pos;
		entity->position.y = entity->position.y;
	}
}

void animate_tube(Game *game, Entity *entity, int f_counter) {
	if (entity->state.active) {
		entity->render.visible = 1;
		entity->position.x = entity->position.x - game->camera_pos;
		entity->position.y = entity->position.y;
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

		entity->render.visible = 1;
		entity->position.x = entity->position.x - game->camera_pos;
		entity->position.y = entity->position.y;
	}
}

void animate_cloud(Game *game, Entity *entity, int f_counter) {
	if (entity->state.active) {


		entity->render.visible = 1;
		entity->position.x = entity->position.x - game->camera_pos;
		entity->position.y = entity->position.y;
		entity->render.pattern_code = ANI_CLOUD_NORMAL;
	}
}

void animate_coin(Game *game, Entity *entity, int f_counter) {
	if (entity->state.active) {
		int frame_count = FRAME_LIMIT;
		int counter = (f_counter >= entity->state.animate_frame_counter) ?
					  f_counter : entity->state.animate_frame_counter + frame_count;
		int rel_counter = counter - entity->state.animate_frame_counter;
		int ani_div;

		if (entity->state.state == STATE_NORMAL) {
			ani_div = (counter / 6) % 4;
		} else {
			ani_div = (counter / 2) % 4;
			if (rel_counter <= 20) {
				entity->position.y += (rel_counter - 20) * 0.17;
			} else if (rel_counter > 35) {
				entity->state.active = 0;
			}
		}

		switch (ani_div) {
			case 0:
				entity->render.pattern_code = ANI_COIN_1;
				break;
			case 1:
				entity->render.pattern_code = ANI_COIN_2;
				break;
			case 2:
				entity->render.pattern_code = ANI_COIN_3;
				break;
			default:
				entity->render.pattern_code = ANI_COIN_4;
		}

		entity->render.visible = 1;
		entity->position.x = entity->position.x - game->camera_pos;
		entity->position.y = entity->position.y;
	}
}

