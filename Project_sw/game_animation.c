#include "game_animation.h"
#include "game_struct.h"


void mario_animation(mario_game *game_0, mario* mario_0, int f_counter){
	int counter, ani_div, rel_counter;
	float dead_acc, dead_v;
	dead_v = -4.7; dead_acc = 0.16;
	counter = (f_counter >= mario_0->animate_frame_counter)? f_counter : f_counter + FRAME_LIMIT;
	rel_counter = counter - mario_0->animate_frame_counter;
	if (mario_0->control_s == MARIO_NORMAL) {
		if (mario_0->y_block == 1){
			if (mario_0->acc_x == 0) mario_0->vis.pattern_code = (mario_0->mario_s == SMALL)? ANI_MARIO_S_NORMAL : ANI_MARIO_L_NORMAL;
			else if ((mario_0->acc_x) * (mario_0->hit.vx) < 0) 
				mario_0->vis.pattern_code = (mario_0->mario_s == SMALL)? ANI_MARIO_S_SHUT : ANI_MARIO_L_SHUT;
			else {
				ani_div = (counter/6)%3;
				switch (ani_div){
					case 0:
						mario_0->vis.pattern_code = (mario_0->mario_s == SMALL)? ANI_MARIO_S_WALK1 : ANI_MARIO_L_WALK1;
						break;
					case 1:
						mario_0->vis.pattern_code = (mario_0->mario_s == SMALL)? ANI_MARIO_S_WALK2 : ANI_MARIO_L_WALK2;
						break;
					case 2:
						mario_0->vis.pattern_code = (mario_0->mario_s == SMALL)? ANI_MARIO_S_WALK3 : ANI_MARIO_L_WALK3;
						break;
					default:
						mario_0->vis.pattern_code = (mario_0->mario_s == SMALL)? ANI_MARIO_S_NORMAL : ANI_MARIO_L_NORMAL;
				}	
			}
		}
		else 
			mario_0->vis.pattern_code = (mario_0->mario_s == SMALL)? ANI_MARIO_S_JUMP : ANI_MARIO_L_JUMP;
	}
	else{
		switch (mario_0->animate_s){
			case HIT:
				ani_div = (rel_counter/3)%2;
				if(rel_counter == 0) mario_0->hit.y += 3;
				else if (rel_counter <= 20) mario_0->vis.pattern_code = (ani_div == 0)? ANI_MARIO_L_HIT : ANI_MARIO_S_HIT;
				else {mario_0->control_s = MARIO_NORMAL; mario_0->mario_s = SMALL; mario_0->hit.sy = 16; mario_0->game_s = MARIO_GMAE_HIT;}
				break;
			case ENLARGE:
				ani_div = (rel_counter/3)%2;
				if(rel_counter == 0) mario_0->hit.y -= 20;
				else if (rel_counter <= 15) mario_0->vis.pattern_code = (ani_div == 0)? ANI_MARIO_S_NORMAL : ANI_MARIO_M_NORMAL;
				else if (rel_counter <= 30) mario_0->vis.pattern_code = (ani_div == 0)? ANI_MARIO_L_NORMAL : ANI_MARIO_M_NORMAL;
				else {mario_0->control_s = MARIO_NORMAL; mario_0->mario_s = LARGE; mario_0->hit.sy = 32;}
				break;
			case ANI_DEAD:
				mario_0->vis.pattern_code =ANI_MARIO_S_DEAD;
				if(rel_counter == 0) {mario_0->hit.vy = dead_v;}
				else if(rel_counter >= 30){
					mario_0->hit.y += mario_0->hit.vy; mario_0->hit.vy += dead_acc;
					if (mario_0->hit.y > 500) mario_0->mario_s = DEAD;
				}
				break;
			default:
				break;
				
		}
	}

	if (rel_counter > 120 && mario_0->game_s == MARIO_GMAE_HIT) {mario_0->game_s = MARIO_GAME_NORMAL;}
	
	mario_0->vis.visible = (mario_0->game_s == MARIO_GAME_NORMAL)? 1 : ((counter/8)%2 == 0)? 1 : 0;
	// game_0.mario_0.vis.flip = 0;
	// game_0.mario_0.vis.pattern_code = 0;
	mario_0->vis.x = mario_0->hit.x - game_0->camera_pos;
	mario_0->vis.y = mario_0->hit.y;
	return;
}
void mush_animation(mario_game *game_0, mush* mush_0, int f_counter){
	int counter, ani_div, rel_counter;
	counter = (f_counter >= mush_0->animate_frame_counter)? f_counter : f_counter + FRAME_LIMIT;
	rel_counter = counter - mush_0->animate_frame_counter;
	if (mush_0->mush_s == MUSH_NORMAL) {
		mush_0->vis.pattern_code = ANI_MUSH_NORMAL;
	}
	else{
		mush_0->vis.pattern_code = ANI_MUSH_NORMAL;
		if (rel_counter < 40) mush_0->hit.y -= 0.5;
		else mush_0->mush_s = MUSH_NORMAL;
	}
	
	// mush_0->vis.visible = 1;
	mush_0->vis.x = mush_0->hit.x - game_0->camera_pos;
	mush_0->vis.y = mush_0->hit.y;
	return;
}
void goomba_animation(mario_game *game_0, goomba* goomba_0, int f_counter){
	int counter, ani_div, rel_counter;
	counter = (f_counter >= goomba_0->animate_frame_counter)? f_counter : f_counter + FRAME_LIMIT;
	rel_counter = counter - goomba_0->animate_frame_counter;
	
	if (goomba_0->goomba_s == GOOMBA_NORMAL){
		ani_div = (counter/7)%2;
		goomba_0->vis.pattern_code = ANI_GOOMBA_NORMAL;
		goomba_0->vis.flip = ani_div;
	} 
	
	else{
		if (rel_counter == 0){ goomba_0->hit.y += 8;}
		else if (rel_counter >= 25){ goomba_0->enable = 0;}
		goomba_0->vis.pattern_code = ANI_GOOMBA_HIT;
	}

	goomba_0->vis.x = goomba_0->hit.x - game_0->camera_pos;
	goomba_0->vis.y = goomba_0->hit.y;
}
void tube_animation(mario_game *game_0, tube* tube_0, int f_counter){
	int counter, ani_div, rel_counter;
	tube_0->vis.visible = 1;
	tube_0->vis.x = tube_0->hit.x - game_0->camera_pos;
	tube_0->vis.y = tube_0->hit.y;
}
void block_animation(mario_game *game_0, block* block_0, int f_counter){
	int counter, ani_div, rel_counter;
	counter = (f_counter >= block_0->animate_frame_counter)? f_counter : f_counter + FRAME_LIMIT;
	rel_counter = counter - block_0->animate_frame_counter;
	switch (block_0->block_t){
		case TYPE_A:
			if (block_0->block_s == BLOCK_NORMAL) {
				block_0->vis.pattern_code = ANI_BLOCK_A1;
			}
			else{
				if (rel_counter <= 8) block_0->hit.y += (rel_counter - 8) * 0.3;
				else if (rel_counter <= 12) block_0->vis.pattern_code = ANI_BLOCK_A_HIT;
				else block_0->enable = 0;
			}
			break;
		case TYPE_B_1:
			block_0->vis.pattern_code = ANI_BLOCK_B;
			break;		
		case TYPE_B_2:
			block_0->vis.pattern_code = ANI_BLOCK_B_V2;
			break;
		case TYPE_B_3:
			block_0->vis.pattern_code = ANI_BLOCK_B_V3;
			break;
		case TYPE_B_4:
			block_0->vis.pattern_code = ANI_BLOCK_B_V4;
			break;
		case TYPE_B_16:
			block_0->vis.pattern_code = ANI_BLOCK_B_16;
			break;
		case TYPE_A_H_8:
			block_0->vis.pattern_code = ANI_BLOCK_A_H8;
			break;

		case OBJ_C:
			ani_div = (counter/8)%4;
			if (block_0->block_s == BLOCK_NORMAL) {
				switch (ani_div){
					case 0:
						block_0->vis.pattern_code = ANI_BLOCK_ITEM1;
						break;
					case 1:
						block_0->vis.pattern_code = ANI_BLOCK_ITEM2;
						break;
					case 2:
						block_0->vis.pattern_code = ANI_BLOCK_ITEM3;
						break;
					default:
						block_0->vis.pattern_code = ANI_BLOCK_ITEM2;
				}
			}
			else{
				if (rel_counter < 4) {block_0->hit.y -= 1; block_0->vis.pattern_code = ANI_BLOCK_ITEM_HIT;}
				else if (rel_counter < 8) {block_0->hit.y += 1; block_0->vis.pattern_code = ANI_BLOCK_ITEM_HIT;}
				else {block_0->block_t = EMP;}
			}
			break;

		case OBJ_M:
			ani_div = (counter/8)%4;
			if (block_0->block_s == BLOCK_NORMAL) {
				switch (ani_div){
					case 0:
						block_0->vis.pattern_code = ANI_BLOCK_ITEM1;
						break;
					case 1:
						block_0->vis.pattern_code = ANI_BLOCK_ITEM2;
						break;
					case 2:
						block_0->vis.pattern_code = ANI_BLOCK_ITEM3;
						break;
					default:
						block_0->vis.pattern_code = ANI_BLOCK_ITEM2;
				}
			}
			else{
				if (rel_counter < 4) {block_0->hit.y -= 1; block_0->vis.pattern_code = ANI_BLOCK_ITEM_HIT;}
				else if (rel_counter < 8) {block_0->hit.y += 1; block_0->vis.pattern_code = ANI_BLOCK_ITEM_HIT;}
				else {block_0->block_t = EMP;}
			}
			break;

		default:
			block_0->vis.pattern_code = ANI_BLOCK_ITEM_EMP;
			break;
	}	
	// block_0->vis.visible = 1;
	block_0->vis.x = block_0->hit.x - game_0->camera_pos;
	block_0->vis.y = block_0->hit.y;
	return;
}
void cloud_animation(mario_game *game_0, cloud* cloud_0, int f_counter){
	int counter, ani_div, rel_counter;
	cloud_0->vis.visible = 1;
	cloud_0->vis.x = cloud_0->hit.x - game_0->camera_pos;
	cloud_0->vis.y = cloud_0->hit.y;
}
void coin_animation(mario_game *game_0, coin* coin_0, int f_counter){
	int counter, ani_div, rel_counter;
	counter = (f_counter >= coin_0->animate_frame_counter)? f_counter : f_counter + FRAME_LIMIT;
	rel_counter = counter - coin_0->animate_frame_counter;
	if (coin_0->coin_s == COIN_NORMAL) {
		ani_div = (counter/6)%4;
	}
	else{
		ani_div = (counter/2)%4;
		if (rel_counter <= 20) coin_0->hit.y += (rel_counter - 20) * 0.17;
		else if (rel_counter > 35) coin_0->enable = 0;
	}

	switch (ani_div){
		case 0:
			coin_0->vis.pattern_code = ANI_COIN_1;
			break;
		case 1:
			coin_0->vis.pattern_code = ANI_COIN_2;
			break;
		case 2:
			coin_0->vis.pattern_code = ANI_COIN_3;
			break;
		default:
			coin_0->vis.pattern_code = ANI_COIN_4;
	}	
	
	// coin_0->vis.visible = 1;
	coin_0->vis.x = coin_0->hit.x - game_0->camera_pos;
	coin_0->vis.y = coin_0->hit.y;
	return;
}
