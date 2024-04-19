#include "game_struct.h"

enum contact hitbox_contact(const hit_box *A, const hit_box *B){
	float AL, AR, AU, AD, BL, BR, BU, BD, dx, dy;
	float ALP, ARP, AUP, ADP;
	float xv_r, yv_r;
	float t_h, t_v;
	int v_c, h_c; // vc pos: DOWN, neg: UP    hc pos RIGHT, neg : LEFT
	// BL = B->x; BR = (B->x) + B->sx; Bu = B->y; BD = (B->y) + B->sy;
	// Zero out
	BL = 0; BR = B->sx; BU = 0; BD = B->sy;
	AL = A->x - B->x; AR = A->x + A->sx - B->x;
	AU = A->y - B->y; AD = A->y + A->sy - B->y; 
	// check if any corner inside before the update 	
	if ((AU > BU && AU < BD && AL > BL && AL < BR) // A left up corner
	||	(AU > BU && AU < BD && AR > BL && AR < BR) // A right up corner
	||	(AD > BU && AD < BD && AL > BL && AL < BR) // A left down corner
	||	(AD > BU && AD < BD && AR > BL && AR < BR) // A right down corner
		){
		return NONE; // Assume no contact	
	}
	else{
		// check if any corner inside AFTER the update 	
		xv_r = A->vx - B->vx; yv_r = A->vy - B->vy; 
		ALP = AL + xv_r; ARP = AR + xv_r; AUP = AU + yv_r; ADP = AD + yv_r;
		t_h = 0; t_v = 0;
		if ((AUP >= BU && AUP <= BD && ALP >= BL && ALP <= BR) // A left up corner
		||	(AUP >= BU && AUP <= BD && ARP >= BL && ARP <= BR) // A right up corner
		||	(ADP >= BU && ADP <= BD && ALP >= BL && ALP <= BR) // A left down corner
		||	(ADP >= BU && ADP <= BD && ARP >= BL && ARP <= BR) // A right down corner
		){
			if (AR <= BL && xv_r != 0) {h_c = 1; dx = BL - AR; t_h = dx/xv_r;}
			else if (AL >= BR  && xv_r != 0) {h_c = -1; dx = BR - AL; t_h = dx/xv_r;}
			else h_c = 0;
			
			if (AD <= BU && yv_r != 0) {v_c = 1; dy = BU -AD; t_v = dy/yv_r;}
			else if (AU >= BD && yv_r != 0) {v_c = -1; dy = BD - AU; t_v = dy/yv_r;}
			else v_c = 0;
			
			if (t_v >= 0 && v_c != 0){
				if (t_h < 0 || h_c == 0){
					return (v_c==1)? DOWN : UP;				
				}
				else if (t_h >= 0 && h_c != 0){
					if (t_v >= t_h){
						return (v_c==1)? DOWN : UP;
					}
					return (h_c==1)? RIGHT : LEFT;
				}
				return (v_c==1)? DOWN : UP;	
			}
		
			else if (t_h >=   0 && h_c != 0){
				return (h_c==1)? RIGHT : LEFT;
			}
			
			return NONE;
			
		}
		else{
			return NONE;		
		}
		return NONE;	
	}
	return NONE;
}

void new_game(mario_game *game){
	// camera
	game->camera_pos = 0;
	//Mario
	game->mario_0.enable = 1;
	game->mario_0.x_block = 0;
	game->mario_0.y_block = 0;
	game->mario_0.hit.sx = 16;
	game->mario_0.hit.sy = 16;
	game->mario_0.hit.x = 128;
	game->mario_0.hit.y = 128;
	game->mario_0.hit.vx = 0;
	game->mario_0.hit.vy = 0;
	game->mario_0.control_s = MARIO_NORMAL;
	game->mario_0.animate_s = HIT;
	game->mario_0.game_s = MARIO_GAME_NORMAL;
	game->mario_0.mario_s = SMALL;
	// Ground
	game->ground_list[0].hit.x = 0;
	game->ground_list[0].hit.y = 368;
	game->ground_list[0].hit.vx = 0;
	game->ground_list[0].hit.vy = 0;
	game->ground_list[0].hit.sx = 448;
	game->ground_list[0].hit.sy = 32;
	
	game->ground_list[1].hit.x = 528;
	game->ground_list[1].hit.y = 368;
	game->ground_list[1].hit.vx = 0;
	game->ground_list[1].hit.vy = 0;
	game->ground_list[1].hit.sx = 944;
	game->ground_list[1].hit.sy = 32;

	game->ground_list[2].hit.x = 1504;
	game->ground_list[2].hit.y = 368;
	game->ground_list[2].hit.vx = 0;
	game->ground_list[2].hit.vy = 0;
	game->ground_list[2].hit.sx = 640;
	game->ground_list[2].hit.sy = 32;

	game->ground_list[3].hit.x = 2176;
	game->ground_list[3].hit.y = 368;
	game->ground_list[3].hit.vx = 0;
	game->ground_list[3].hit.vy = 0;
	game->ground_list[3].hit.sx = 800;
	game->ground_list[3].hit.sy = 32;

	//Block
	int b_i;
	for (b_i = 0; b_i < BLOCK_NUM; b_i++){
		game->block_list[b_i].hit.x = 0;
		game->block_list[b_i].hit.y = 0;
		game->block_list[b_i].hit.vx = 0;
		game->block_list[b_i].hit.vy = 0;
		game->block_list[b_i].hit.sx = 16;
		game->block_list[b_i].hit.sy = 16;
		game->block_list[b_i].enable = 0;
		game->block_list[b_i].loaded = 0;
		game->block_list[b_i].L = 0;
		game->block_list[b_i].R = 0;
		game->block_list[b_i].U = 0;
		game->block_list[b_i].D = 0;
	}	
	
	game->block_list[0].hit.x = 320;
	game->block_list[0].hit.y = 304;
	game->block_list[0].hit.vx = 0;
	game->block_list[0].hit.vy = 0;
	game->block_list[0].hit.sx = 16;
	game->block_list[0].hit.sy = 16;
	game->block_list[0].enable = 1;
	game->block_list[0].loaded = 0;
	game->block_list[0].L = 1;
	game->block_list[0].R = 0;
	game->block_list[0].U = 0;
	game->block_list[0].D = 0;
	game->block_list[0].block_t = TYPE_A;
	game->block_list[0].block_s = BLOCK_NORMAL;
	game->block_list[0].vis.visible = 1;

	game->block_list[1].hit.x = 304;
	game->block_list[1].hit.y = 304;
	game->block_list[1].hit.vx = 0;
	game->block_list[1].hit.vy = 0;
	game->block_list[1].hit.sx = 16;
	game->block_list[1].hit.sy = 16;
	game->block_list[1].enable = 1;
	game->block_list[1].loaded = 0;
	game->block_list[1].L = 1;
	game->block_list[1].R = 1;
	game->block_list[1].U = 0;
	game->block_list[1].D = 0;
	game->block_list[1].block_t = OBJ_C;
	game->block_list[1].block_s = BLOCK_NORMAL;
	game->block_list[1].vis.visible = 1;

	game->block_list[2].hit.x = 288;
	game->block_list[2].hit.y = 304;
	game->block_list[2].hit.vx = 0;
	game->block_list[2].hit.vy = 0;
	game->block_list[2].hit.sx = 16;
	game->block_list[2].hit.sy = 16;
	game->block_list[2].enable = 1;
	game->block_list[2].loaded =0;
	game->block_list[2].L = 1;
	game->block_list[2].R = 1;
	game->block_list[2].U = 0;
	game->block_list[2].D = 0;
	game->block_list[2].block_t = TYPE_A;
	game->block_list[2].block_s = BLOCK_NORMAL;
	game->block_list[2].vis.visible = 1;

	game->block_list[3].hit.x = 272;
	game->block_list[3].hit.y = 304;
	game->block_list[3].hit.vx = 0;
	game->block_list[3].hit.vy = 0;
	game->block_list[3].hit.sx = 16;
	game->block_list[3].hit.sy = 16;
	game->block_list[3].enable = 1;
	game->block_list[3].loaded = 0;
	game->block_list[3].L = 0;
	game->block_list[3].R = 1;
	game->block_list[3].U = 0;
	game->block_list[3].D = 0;
	game->block_list[3].block_t = OBJ_M;
	game->block_list[3].block_s = BLOCK_NORMAL;
	game->block_list[3].vis.visible = 1;

	game->block_list[4].hit.x = 224;
	game->block_list[4].hit.y = 304;
	game->block_list[4].hit.vx = 0;
	game->block_list[4].hit.vy = 0;
	game->block_list[4].hit.sx = 16;
	game->block_list[4].hit.sy = 16;
	game->block_list[4].enable = 1;
	game->block_list[4].loaded = 0;
	game->block_list[4].L = 0;
	game->block_list[4].R = 0;
	game->block_list[4].U = 0;
	game->block_list[4].D = 0;
	game->block_list[4].block_t = TYPE_A;
	game->block_list[4].block_s = BLOCK_NORMAL;
	game->block_list[4].vis.visible = 1;



	game->block_list[5].hit.x = 560 + 48;
	game->block_list[5].hit.y = 352;
	game->block_list[5].hit.vx = 0;
	game->block_list[5].hit.vy = 0;
	game->block_list[5].hit.sx = 16;
	game->block_list[5].hit.sy = 16;
	game->block_list[5].enable = 1;
	game->block_list[5].loaded = 0;
	game->block_list[5].L = 0;
	game->block_list[5].R = 0;
	game->block_list[5].U = 0;
	game->block_list[5].D = 0;
	game->block_list[5].block_t = TYPE_B_1;
	game->block_list[5].block_s = BLOCK_NORMAL;
	game->block_list[5].vis.visible = 1;

	game->block_list[6].hit.x = 576 + 48;
	game->block_list[6].hit.y = 336;
	game->block_list[6].hit.vx = 0;
	game->block_list[6].hit.vy = 0;
	game->block_list[6].hit.sx = 16;
	game->block_list[6].hit.sy = 32;
	game->block_list[6].enable = 1;
	game->block_list[6].loaded = 0;
	game->block_list[6].L = 0;
	game->block_list[6].R = 0;
	game->block_list[6].U = 0;
	game->block_list[6].D = 0;
	game->block_list[6].block_t = TYPE_B_2;
	game->block_list[6].block_s = BLOCK_NORMAL;
	game->block_list[6].vis.visible = 1;

	game->block_list[7].hit.x = 592 + 48;
	game->block_list[7].hit.y = 320;
	game->block_list[7].hit.vx = 0;
	game->block_list[7].hit.vy = 0;
	game->block_list[7].hit.sx = 16;
	game->block_list[7].hit.sy = 48;
	game->block_list[7].enable = 1;
	game->block_list[7].loaded =0;
	game->block_list[7].L = 0;
	game->block_list[7].R = 0;
	game->block_list[7].U = 0;
	game->block_list[7].D = 0;
	game->block_list[7].block_t = TYPE_B_3;
	game->block_list[7].block_s = BLOCK_NORMAL;
	game->block_list[7].vis.visible = 1;

	game->block_list[8].hit.x = 608 + 48;
	game->block_list[8].hit.y = 304;
	game->block_list[8].hit.vx = 0;
	game->block_list[8].hit.vy = 0;
	game->block_list[8].hit.sx = 16;
	game->block_list[8].hit.sy = 64;
	game->block_list[8].enable = 1;
	game->block_list[8].loaded = 0;
	game->block_list[8].L = 0;
	game->block_list[8].R = 0;
	game->block_list[8].U = 0;
	game->block_list[8].D = 0;
	game->block_list[8].block_t = TYPE_B_4;
	game->block_list[8].block_s = BLOCK_NORMAL;
	game->block_list[8].vis.visible = 1;

	game->block_list[9].hit.x = 688 + 48;
	game->block_list[9].hit.y = 256;
	game->block_list[9].hit.vx = 0;
	game->block_list[9].hit.vy = 0;
	game->block_list[9].hit.sx = 128;
	game->block_list[9].hit.sy = 16;
	game->block_list[9].enable = 1;
	game->block_list[9].loaded = 0;
	game->block_list[9].L = 0;
	game->block_list[9].R = 0;
	game->block_list[9].U = 0;
	game->block_list[9].D = 0;
	game->block_list[9].block_t = TYPE_A_H_8;
	game->block_list[9].block_s = BLOCK_NORMAL;
	game->block_list[9].vis.visible = 1;

	game->block_list[10].hit.x = 672 + 48;
	game->block_list[10].hit.y = 176;
	game->block_list[10].hit.vx = 0;
	game->block_list[10].hit.vy = 0;
	game->block_list[10].hit.sx = 16;
	game->block_list[10].hit.sy = 16;
	game->block_list[10].enable = 1;
	game->block_list[10].loaded = 0;
	game->block_list[10].L = 0;
	game->block_list[10].R = 0;
	game->block_list[10].U = 0;
	game->block_list[10].D = 0;
	game->block_list[10].block_t = OBJ_C;
	game->block_list[10].block_s = BLOCK_NORMAL;
	game->block_list[10].vis.visible = 1;

	game->block_list[11].hit.x = 736 + 48;
	game->block_list[11].hit.y = 176;
	game->block_list[11].hit.vx = 0;
	game->block_list[11].hit.vy = 0;
	game->block_list[11].hit.sx = 16;
	game->block_list[11].hit.sy = 16;
	game->block_list[11].enable = 1;
	game->block_list[11].loaded = 0;
	game->block_list[11].L = 0;
	game->block_list[11].R = 0;
	game->block_list[11].U = 0;
	game->block_list[11].D = 0;
	game->block_list[11].block_t = OBJ_C;
	game->block_list[11].block_s = BLOCK_NORMAL;
	game->block_list[11].vis.visible = 1;

	game->block_list[12].hit.x = 800 + 48;
	game->block_list[12].hit.y = 176;
	game->block_list[12].hit.vx = 0;
	game->block_list[12].hit.vy = 0;
	game->block_list[12].hit.sx = 16;
	game->block_list[12].hit.sy = 16;
	game->block_list[12].enable = 1;
	game->block_list[12].loaded = 0;
	game->block_list[12].L = 0;
	game->block_list[12].R = 0;
	game->block_list[12].U = 0;
	game->block_list[12].D = 0;
	game->block_list[12].block_t = OBJ_C;
	game->block_list[12].block_s = BLOCK_NORMAL;
	game->block_list[12].vis.visible = 1;

	game->block_list[13].hit.x = 736 + 48;
	game->block_list[13].hit.y = 304;
	game->block_list[13].hit.vx = 0;
	game->block_list[13].hit.vy = 0;
	game->block_list[13].hit.sx = 16;
	game->block_list[13].hit.sy = 16;
	game->block_list[13].enable = 1;
	game->block_list[13].loaded = 0;
	game->block_list[13].L = 0;
	game->block_list[13].R = 0;
	game->block_list[13].U = 0;
	game->block_list[13].D = 0;
	game->block_list[13].block_t = OBJ_M;
	game->block_list[13].block_s = BLOCK_NORMAL;
	game->block_list[13].vis.visible = 1;

	game->block_list[14].hit.x = 1104 + 48;
	game->block_list[14].hit.y = 272;
	game->block_list[14].hit.vx = 0;
	game->block_list[14].hit.vy = 0;
	game->block_list[14].hit.sx = 16;
	game->block_list[14].hit.sy = 48;
	game->block_list[14].enable = 1;
	game->block_list[14].loaded = 0;
	game->block_list[14].L = 0;
	game->block_list[14].R = 0;
	game->block_list[14].U = 0;
	game->block_list[14].D = 0;
	game->block_list[14].block_t = TYPE_B_3;
	game->block_list[14].block_s = BLOCK_NORMAL;
	game->block_list[14].vis.visible = 1;

	game->block_list[15].hit.x = 1120 + 48;
	game->block_list[15].hit.y = 304;
	game->block_list[15].hit.vx = 0;
	game->block_list[15].hit.vy = 0;
	game->block_list[15].hit.sx = 16;
	game->block_list[15].hit.sy = 16;
	game->block_list[15].enable = 1;
	game->block_list[15].loaded = 0;
	game->block_list[15].L = 1;
	game->block_list[15].R = 1;
	game->block_list[15].U = 0;
	game->block_list[15].D = 0;
	game->block_list[15].block_t = TYPE_A;
	game->block_list[15].block_s = BLOCK_NORMAL;
	game->block_list[15].vis.visible = 1;

	game->block_list[16].hit.x = 1136 + 48;
	game->block_list[16].hit.y = 304;
	game->block_list[16].hit.vx = 0;
	game->block_list[16].hit.vy = 0;
	game->block_list[16].hit.sx = 16;
	game->block_list[16].hit.sy = 16;
	game->block_list[16].enable = 1;
	game->block_list[16].loaded = 0;
	game->block_list[16].L = 1;
	game->block_list[16].R = 1;
	game->block_list[16].U = 0;
	game->block_list[16].D = 0;
	game->block_list[16].block_t = TYPE_A;
	game->block_list[16].block_s = BLOCK_NORMAL;
	game->block_list[16].vis.visible = 1;

	game->block_list[17].hit.x = 1152 + 48;
	game->block_list[17].hit.y = 272;
	game->block_list[17].hit.vx = 0;
	game->block_list[17].hit.vy = 0;
	game->block_list[17].hit.sx = 16;
	game->block_list[17].hit.sy = 48;
	game->block_list[17].enable = 1;
	game->block_list[17].loaded = 0;
	game->block_list[17].L = 0;
	game->block_list[17].R = 0;
	game->block_list[17].U = 0;
	game->block_list[17].D = 0;
	game->block_list[17].block_t = TYPE_B_3;
	game->block_list[17].block_s = BLOCK_NORMAL;
	game->block_list[17].vis.visible = 1;
	// Sort block with block x accending 
	int i, j;
	block temp;
	for (i = 0; i < BLOCK_NUM - 1; i++)
		for (j = 0; j < BLOCK_NUM - 1 - i; j++) {
			if (game->block_list[j].hit.x > game->block_list[j + 1].hit.x) {
		   		temp = game->block_list[j];
		        game->block_list[j] = game->block_list[j + 1];
		        game->block_list[j + 1] = temp;
		     }
		  }
	for (i = 0; i < BLOCK_NUM; i++)
		printf("%d: %f \n", i, game->block_list[i].hit.x);
	
	// Coin
	int c_i;
	for (c_i = 0; c_i < COIN_NUM; c_i++){
		game->coin_list[c_i].hit.x = 0;
		game->coin_list[c_i].hit.y = 0;
		game->coin_list[c_i].hit.vx = 0;
		game->coin_list[c_i].hit.vy = 0;
		game->coin_list[c_i].hit.sx = 8;
		game->coin_list[c_i].hit.sy = 16;
		game->coin_list[c_i].enable = 0;
		game->coin_list[c_i].loaded = 0;
	}	
	
	game->coin_list[0].hit.x = 324;
	game->coin_list[0].hit.y = 352;
	game->coin_list[0].hit.vx = 0;
	game->coin_list[0].hit.vy = 0;
	game->coin_list[0].hit.sx = 8;
	game->coin_list[0].hit.sy = 16;
	game->coin_list[0].enable = 1;
	game->coin_list[0].loaded = 1;
	game->coin_list[0].coin_s = COIN_NORMAL;

	game->coin_list[1].hit.x = 964;
	game->coin_list[1].hit.y = 352;
	game->coin_list[1].hit.vx = 0;
	game->coin_list[1].hit.vy = 0;
	game->coin_list[1].hit.sx = 8;
	game->coin_list[1].hit.sy = 16;
	game->coin_list[1].enable = 1;
	game->coin_list[1].loaded = 1;
	game->coin_list[1].coin_s = COIN_NORMAL;

	game->coin_list[2].hit.x = 324;
	game->coin_list[2].hit.y = 288;
	game->coin_list[2].hit.vx = 0;
	game->coin_list[2].hit.vy = 0;
	game->coin_list[2].hit.sx = 8;
	game->coin_list[2].hit.sy = 16;
	game->coin_list[2].enable = 1;
	game->coin_list[2].loaded = 1;
	game->coin_list[2].coin_s = COIN_NORMAL;

	game->coin_list[3].hit.x = 656 + 48;
	game->coin_list[3].hit.y = 352;
	game->coin_list[3].hit.vx = 0;
	game->coin_list[3].hit.vy = 0;
	game->coin_list[3].hit.sx = 8;
	game->coin_list[3].hit.sy = 16;
	game->coin_list[3].enable = 1;
	game->coin_list[3].loaded = 1;
	game->coin_list[3].coin_s = COIN_NORMAL;

	game->coin_list[4].hit.x = 704 + 48;
	game->coin_list[4].hit.y = 352;
	game->coin_list[4].hit.vx = 0;
	game->coin_list[4].hit.vy = 0;
	game->coin_list[4].hit.sx = 8;
	game->coin_list[4].hit.sy = 16;
	game->coin_list[4].enable = 1;
	game->coin_list[4].loaded = 1;
	game->coin_list[4].coin_s = COIN_NORMAL;

	game->coin_list[5].hit.x = 1124 + 48;
	game->coin_list[5].hit.y = 288;
	game->coin_list[5].hit.vx = 0;
	game->coin_list[5].hit.vy = 0;
	game->coin_list[5].hit.sx = 8;
	game->coin_list[5].hit.sy = 16;
	game->coin_list[5].enable = 1;
	game->coin_list[5].loaded = 1;
	game->coin_list[5].coin_s = COIN_NORMAL;

	game->coin_list[6].hit.x = 1140 + 48;
	game->coin_list[6].hit.y = 288;
	game->coin_list[6].hit.vx = 0;
	game->coin_list[6].hit.vy = 0;
	game->coin_list[6].hit.sx = 8;
	game->coin_list[6].hit.sy = 16;
	game->coin_list[6].enable = 1;
	game->coin_list[6].loaded = 1;
	game->coin_list[6].coin_s = COIN_NORMAL;

	// Mush
	int m_i;
	for (m_i = 0; m_i < MUSH_NUM; m_i++){
		game->mush_list[m_i].hit.x = 0;
		game->mush_list[m_i].hit.y = 0;
		game->mush_list[m_i].hit.vx = 0;
		game->mush_list[m_i].hit.vy = 0;
		game->mush_list[m_i].hit.sx = 16;
		game->mush_list[m_i].hit.sy = 16;
		game->mush_list[m_i].enable = 0;
		game->mush_list[m_i].loaded = 0;
	}	


	// Goomba
	int g_i;
	for (g_i = 0; g_i < GOOMBA_NUM; g_i++){
		game->goomba_list[g_i].hit.x = 0;
		game->goomba_list[g_i].hit.y = 0;
		game->goomba_list[g_i].hit.vx = 0;
		game->goomba_list[g_i].hit.vy = 0;
		game->goomba_list[g_i].hit.sx = 16;
		game->goomba_list[g_i].hit.sy = 16;
		game->goomba_list[g_i].enable = 0;
		game->goomba_list[g_i].loaded = 0;
	}	
	
	game->goomba_list[0].hit.x = 324;
	game->goomba_list[0].hit.y = 268;
	game->goomba_list[0].hit.vx = 0;
	game->goomba_list[0].hit.vy = 0;
	game->goomba_list[0].hit.sx = 16;
	game->goomba_list[0].hit.sy = 16;
	game->goomba_list[0].enable = 1;
	game->goomba_list[0].loaded = 1;
	game->goomba_list[0].goomba_s = GOOMBA_NORMAL;
	game->goomba_list[0].l_limit = 300;
	game->goomba_list[0].r_limit = 600;

	game->goomba_list[1].hit.x = 720 + 48;
	game->goomba_list[1].hit.y = 208;
	game->goomba_list[1].hit.vx = 0;
	game->goomba_list[1].hit.vy = 0;
	game->goomba_list[1].hit.sx = 16;
	game->goomba_list[1].hit.sy = 16;
	game->goomba_list[1].enable = 1;
	game->goomba_list[1].loaded = 1;
	game->goomba_list[1].goomba_s = GOOMBA_NORMAL;
	game->goomba_list[1].l_limit = 704;
	game->goomba_list[1].r_limit = 800;

	game->goomba_list[2].hit.x = 1124 + 48;
	game->goomba_list[2].hit.y = 272;
	game->goomba_list[2].hit.vx = 0;
	game->goomba_list[2].hit.vy = 0;
	game->goomba_list[2].hit.sx = 16;
	game->goomba_list[2].hit.sy = 16;
	game->goomba_list[2].enable = 1;
	game->goomba_list[2].loaded = 1;
	game->goomba_list[2].goomba_s = GOOMBA_NORMAL;
	game->goomba_list[2].l_limit = 1060;
	game->goomba_list[2].r_limit = 1204;

	// Tube
	int t_i;
	for (t_i = 0; t_i < TUBE_NUM; t_i++){
		game->tube_list[t_i].hit.x = 0;
		game->tube_list[t_i].hit.y = 0;
		game->tube_list[t_i].hit.vx = 0;
		game->tube_list[t_i].hit.vy = 0;
		game->tube_list[t_i].hit.sx = 32;
		game->tube_list[t_i].hit.sy = 160;
		game->tube_list[t_i].enable = 0;
		game->tube_list[t_i].loaded = 0;
	}	
	
	game->tube_list[0].hit.x = 416;
	game->tube_list[0].hit.y = 320;
	game->tube_list[0].hit.vx = 0;
	game->tube_list[0].hit.vy = 0;
	game->tube_list[0].hit.sx = 32;
	game->tube_list[0].hit.sy = 160;
	game->tube_list[0].enable = 1;
	game->tube_list[0].loaded = 1;

	// Cloud
	for (t_i = 0; t_i < CLOUD_NUM; t_i++){
		game->cloud_list[t_i].hit.x = 0;
		game->cloud_list[t_i].hit.y = 0;
		game->cloud_list[t_i].hit.vx = 0;
		game->cloud_list[t_i].hit.vy = 0;
		game->cloud_list[t_i].hit.sx = 0;
		game->cloud_list[t_i].hit.sy = 0;
		game->cloud_list[t_i].enable = 0;
		game->cloud_list[t_i].loaded = 0;
	}	
	
	game->cloud_list[0].hit.x = 240;
	game->cloud_list[0].hit.y = 176;
	game->cloud_list[0].hit.vx = 0;
	game->cloud_list[0].hit.vy = 0;
	game->cloud_list[0].hit.sx = 0;
	game->cloud_list[0].hit.sy = 0;
	game->cloud_list[0].enable = 1;
	game->cloud_list[0].loaded = 1;

	game->cloud_list[1].hit.x = 240 + CAMERA_SIZE + 60 + 48;
	game->cloud_list[1].hit.y = 144;
	game->cloud_list[1].hit.vx = 0;
	game->cloud_list[1].hit.vy = 0;
	game->cloud_list[1].hit.sx = 0;
	game->cloud_list[1].hit.sy = 0;
	game->cloud_list[1].enable = 1;
	game->cloud_list[1].loaded = 1;

	
}
