/*
 * Game Logic
 * First Edied on April 15 2023 by Zhiyuan Liu
*/

#include <stdio.h>
#include <math.h>
#include "vga_ball.h"
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "usbkeyboard.h"
#include <pthread.h>
#include "game_struct.h"
#include "game_animation.h"

#define PPU_ADDR 0

// Ground space
#define GROUND_0_L 448
#define GROUND_0_R 527
#define GROUND_1_L 1472
#define GROUND_1_R 1503
#define GROUND_2_L 2144
#define GROUND_2_R 2175
// Keyboard // joystic input
enum key_input{KEY_NONE, KEY_JUMP, KEY_LEFT, KEY_RIGHT, KEY_NEWGAME, KEY_END};
enum key_input current_key;
// Sound Effect
#define SOUND_JUMP 0
#define SOUND_COIN 0
#define SOUND_BLOCK 0
#define SOUND_DEAD 0
#define SOUND_NONE 5
// Avalon bus file ind
int vga_ball_fd;

int info_001 = 1;
int info_010 = 2;
int info_011 = 3;
int info_100 = 4;

int block_r = 0;
int block_l = 0;
int frame_counter = 0;

int sound_new = 0;
int sound_ind = 0;


void write_2_hw(int addr, int info)
{
	vga_ball_arg_t vla;
	vla.addr = addr;
	vla.info = info;
	if (ioctl(vga_ball_fd, VGA_BALL_WRITE_BACKGROUND, &vla)) {
      		perror("ioctl(VGA_BALL_SET_BACKGROUND) failed");
      		return;
	}
}

void flush_mario(const mario *mario_0, int ping_pong){
	int visible = mario_0->vis.visible;
	int flip = mario_0->vis.flip;
	int x = mario_0->vis.x;
	int y = mario_0->vis.y;
	int pp = mario_0->vis.pattern_code;
	write_2_hw(0, (int)((1 << 26) + (1 << 17) + (info_001 << 14) + (ping_pong << 13) + (visible << 12) + (flip << 11) + (pp & 0x1F)));
	write_2_hw(0, (int)((1 << 26) + (1 << 17) + (info_010 << 14) + (ping_pong << 13) + (x & 0x3FF)));
	write_2_hw(0, (int)((1 << 26) + (1 << 17) + (info_011 << 14) + (ping_pong << 13) + (y & 0x3FF)));
}
void flush_mush(const mush *mush_0, int ping_pong){
	int visible = mush_0->vis.visible;
	int flip = mush_0->vis.flip;
	int x = mush_0->vis.x;
	int y = mush_0->vis.y;
	int pp = mush_0->vis.pattern_code;
	write_2_hw(0, (int)((9 << 26) + ((0&0x1F) << 21) + (1 << 17) + (info_001 << 14) + (ping_pong << 13) + (1 << 12) + (0 << 11) + (pp & 0x1F)));
	write_2_hw(0, (int)((9 << 26) + ((0&0x1F) << 21) + (1 << 17) + (info_010 << 14) + (ping_pong << 13) + (x & 0x3FF)));
	write_2_hw(0, (int)((9 << 26) + ((0&0x1F) << 21) + (1 << 17) + (info_011 << 14) + (ping_pong << 13) + (y & 0x3FF)));
}
void flush_goomba(const goomba *goomba_0, int ind, int ping_pong){
	int visible = goomba_0->vis.visible;
	int flip = goomba_0->vis.flip;
	int x = goomba_0->vis.x;
	int y = goomba_0->vis.y;
	int pp = goomba_0->vis.pattern_code;
	write_2_hw(0, (int)((5 << 26) + ((0&0x1F) << 21) + (1 << 17) + (info_001 << 14) + (ping_pong << 13) + (1 << 12) + (flip << 11) + (pp & 0x1F)));
	write_2_hw(0, (int)((5 << 26) + ((0&0x1F) << 21) + (1 << 17) + (info_010 << 14) + (ping_pong << 13) + (x & 0x3FF)));
	write_2_hw(0, (int)((5 << 26) + ((0&0x1F) << 21) + (1 << 17) + (info_011 << 14) + (ping_pong << 13) + (y & 0x3FF)));
}
void flush_block(const block *block_0, int ind, int ping_pong){
	int visible = block_0->vis.visible;
	int flip = block_0->vis.flip;
	int x = block_0->vis.x;
	int y = block_0->vis.y;
	int pp = block_0->vis.pattern_code;
	write_2_hw(0, (int)((2 << 26) + ((ind&0x1F) << 21) + (1 << 17) + (info_001 << 14) + (ping_pong << 13) + (1 << 12) + (0 << 11) + (pp & 0x1F)));
	write_2_hw(0, (int)((2 << 26) + ((ind&0x1F) << 21) + (1 << 17) + (info_010 << 14) + (ping_pong << 13) + (x & 0x3FF)));
	write_2_hw(0, (int)((2 << 26) + ((ind&0x1F) << 21) + (1 << 17) + (info_011 << 14) + (ping_pong << 13) + (y & 0x3FF)));
}
// Flush the ground info into PPU (These are the hole locations left and right)
void flush_ground(int camera_pos, int ping_pong){
	int ground_r[3] = {GROUND_0_R, GROUND_1_R, GROUND_2_R};
	int ground_l[3] = {GROUND_0_L, GROUND_1_L, GROUND_2_L};
	int ground_screen_l = 0;
	int ground_screen_r = 0;
	for (int i = 0; i < GROUND_NUM; i++){
		if ((ground_l[i]<=camera_pos + CAMERA_SIZE + LOAD_LIMIT && ground_l[i] >= camera_pos) ||
			(ground_r[i]<=camera_pos + CAMERA_SIZE + LOAD_LIMIT && ground_r[i] >= camera_pos)){
			ground_screen_r = (ground_r[i]-camera_pos <= CAMERA_SIZE + LOAD_LIMIT)? ground_r[i]-camera_pos : CAMERA_SIZE + LOAD_LIMIT;
			ground_screen_l = (ground_l[i] >= camera_pos)? ground_l[i]-camera_pos : 0;
			break;
		} 
	}
	
	write_2_hw(0, (int)((15 << 26) + ((0&0x1F) << 21) + (1 << 17) + (info_001 << 14) + (ping_pong << 13) + (1 << 12) + (0 << 11) + (0 & 0x1F)));
	write_2_hw(0, (int)((15 << 26) + ((0&0x1F) << 21) + (1 << 17) + (info_010 << 14) + (ping_pong << 13) + ((15 - (camera_pos%16)) & 0x3FF)));
	write_2_hw(0, (int)((15 << 26) + ((0&0x1F) << 21) + (1 << 17) + (info_011 << 14) + (ping_pong << 13) + (ground_screen_l & 0x3FF)));
	write_2_hw(0, (int)((15 << 26) + ((0&0x1F) << 21) + (1 << 17) + (info_100 << 14) + (ping_pong << 13) + (ground_screen_r & 0x3FF)));
}
void flush_tube(const tube *tube_0, int ping_pong){
	int visible = tube_0->vis.visible;
	int flip = tube_0->vis.flip;
	int x = tube_0->vis.x;
	int y = tube_0->vis.y;

	write_2_hw(0, (int)((10 << 26) + ((0&0x1F) << 21) + (1 << 17) + (info_001 << 14) + (ping_pong << 13) + (1 << 12) + (0 << 11) + (ANI_TUBE_H & 0x1F)));
	write_2_hw(0, (int)((10 << 26) + ((0&0x1F) << 21) + (1 << 17) + (info_010 << 14) + (ping_pong << 13) + (x & 0x3FF)));
	write_2_hw(0, (int)((10 << 26) + ((0&0x1F) << 21) + (1 << 17) + (info_011 << 14) + (ping_pong << 13) + (y & 0x3FF)));

	write_2_hw(0, (int)((10 << 26) + ((1&0x1F) << 21) + (1 << 17) + (info_001 << 14) + (ping_pong << 13) + (1 << 12) + (0 << 11) + (ANI_TUBE_B & 0x1F)));
	write_2_hw(0, (int)((10 << 26) + ((1&0x1F) << 21) + (1 << 17) + (info_010 << 14) + (ping_pong << 13) + (x & 0x3FF)));
	write_2_hw(0, (int)((10 << 26) + ((1&0x1F) << 21) + (1 << 17) + (info_011 << 14) + (ping_pong << 13) + (y & 0x3FF)));
}
void flush_cloud(const cloud *cloud_0, int ping_pong){
	int visible = cloud_0->vis.visible;
	int flip = cloud_0->vis.flip;
	int x = cloud_0->vis.x;
	int y = cloud_0->vis.y;

	write_2_hw(0, (int)((14 << 26) + ((0&0x1F) << 21) + (1 << 17) + (info_001 << 14) + (ping_pong << 13) + (1 << 12) + (0 << 11) + (ANI_CLOUD_NORMAL & 0x1F)));
	write_2_hw(0, (int)((14 << 26) + ((0&0x1F) << 21) + (1 << 17) + (info_010 << 14) + (ping_pong << 13) + (x & 0x3FF)));
	write_2_hw(0, (int)((14 << 26) + ((0&0x1F) << 21) + (1 << 17) + (info_011 << 14) + (ping_pong << 13) + (y & 0x3FF)));

}
void flush_coin(const coin *coin_0, int ind, int ping_pong){
	int visible = coin_0->vis.visible;
	int flip = coin_0->vis.flip;
	int x = coin_0->vis.x;
	int y = coin_0->vis.y;
	int pp = coin_0->vis.pattern_code;
	write_2_hw(0, (int)((3 << 26) + ((ind&0x1F) << 21) + (1 << 17) + (info_001 << 14) + (ping_pong << 13) + (1 << 12) + (0 << 11) + (pp & 0x1F)));
	write_2_hw(0, (int)((3 << 26) + ((ind&0x1F) << 21) + (1 << 17) + (info_010 << 14) + (ping_pong << 13) + (x & 0x3FF)));
	write_2_hw(0, (int)((3 << 26) + ((ind&0x1F) << 21) + (1 << 17) + (info_011 << 14) + (ping_pong << 13) + (y & 0x3FF)));
}
void flush_ping_pong_frame(const mario_game *game_0, int ping_pong){

	int block_count = 0;
	int coin_count = 0;
	int mush_count = 0;
	int goomba_count = 0;
	int tube_count = 0;
	int cloud_count = 0;
	// Flush Mario	
	mario_animation(game_0, &(game_0->mario_0), frame_counter);
	flush_mario(&(game_0->mario_0), ping_pong);

	// Flush Ground
	flush_ground(game_0->camera_pos, ping_pong);

	// Flush Block
	// ******LATER ADD A LIMITED NUMBER OF BLOCK
	for(int i = block_l; (i < block_r) && (block_count < BLOCK_VIS_LIMIT); i++){
		if(game_0->block_list[i].vis.visible == 1 && game_0->block_list[i].enable == 1){
			block_animation(game_0, &(game_0->block_list[i]), frame_counter);
			flush_block(&(game_0->block_list[i]), block_count, ping_pong);
			block_count += 1;
		}
	}
	// Flush Coin
	for(int i = 0; (i < COIN_NUM) && (coin_count < COIN_VIS_LIMIT); i++){
		if(game_0->coin_list[i].enable == 1 && game_0->coin_list[i].hit.x >= game_0-> camera_pos
		&& game_0->coin_list[i].hit.x <= game_0->camera_pos + CAMERA_SIZE + LOAD_LIMIT){
			coin_animation(game_0, &(game_0->coin_list[i]), frame_counter);
			flush_coin(&(game_0->coin_list[i]), coin_count, ping_pong);
			coin_count += 1;
		}
	}

	// Flush Mush
	for(int i = 0; (i < MUSH_NUM) && (mush_count < MUSH_VIS_LIMIT); i++){
		if(game_0->mush_list[i].enable == 1 && game_0->mush_list[i].hit.x >= game_0-> camera_pos
		&& game_0->mush_list[i].hit.x <= game_0->camera_pos + CAMERA_SIZE + LOAD_LIMIT){
			mush_animation(game_0, &(game_0->mush_list[i]), frame_counter);
			flush_mush(&(game_0->mush_list[i]), ping_pong);
			mush_count += 1;
		}
	}

	// Flush Goomba
	for(int i = 0; (i < GOOMBA_NUM) && (goomba_count < GOOMBA_VIS_LIMIT); i++){
		if(game_0->goomba_list[i].enable == 1 && game_0->goomba_list[i].hit.x >= game_0-> camera_pos
		&& game_0->goomba_list[i].hit.x <= game_0->camera_pos + CAMERA_SIZE + LOAD_LIMIT){
			goomba_animation(game_0, &(game_0->goomba_list[i]), frame_counter);
			flush_goomba(&(game_0->goomba_list[i]), goomba_count, ping_pong);
			goomba_count += 1;
		}
	}

	// Flush Tube
	for(int i = 0; (i < TUBE_NUM) && (tube_count < TUBE_VIS_LIMIT); i++){
		if(game_0->tube_list[i].enable == 1 && game_0->tube_list[i].hit.x >= game_0-> camera_pos
		&& game_0->tube_list[i].hit.x <= game_0->camera_pos + CAMERA_SIZE + LOAD_LIMIT){
			tube_animation(game_0, &(game_0->tube_list[i]), frame_counter);
			flush_tube(&(game_0->tube_list[i]), ping_pong);
			tube_count += 1;
		}
	}

	// Flush Cloud
	for(int i = 0; (i <  CLOUD_NUM) && (cloud_count < CLOUD_VIS_LIMIT); i++){
		if(game_0->cloud_list[i].enable == 1 && game_0->cloud_list[i].hit.x >= game_0-> camera_pos
		&& game_0->cloud_list[i].hit.x <= game_0->camera_pos + CAMERA_SIZE + LOAD_LIMIT){
			cloud_animation(game_0, &(game_0->cloud_list[i]), frame_counter);
			flush_cloud(&(game_0->cloud_list[i]), ping_pong);
			cloud_count += 1;
		}
	}


	// Flush the current frame
	write_2_hw(0, (int)((1 << 26) + (0xf << 17) + (ping_pong << 13)));
	frame_counter = (frame_counter >= FRAME_LIMIT) ? 0 : frame_counter + 1;

	// Flush the sound effect
	if (sound_new == 1){
		write_2_hw(4, (int)(SOUND_NONE));
		sound_new = 0;
	}
	else{
		write_2_hw(4, (int)(sound_ind));	
	}
	
}


//Keyboard
// input from device
// libusb_context *ctx = NULL; // a libusb session
// libusb_device **devs;       // pointer to pointer of device, used to retrieve a list of devices
// int r;                      // for return values
// ssize_t cnt;                // holding number of devices in list
// struct libusb_device_handle *mouse;   // a mouse device handle
struct libusb_device_handle *keyboard;

uint8_t endpoint_address;               // USB endpoint address for keyboard

// Threads================
pthread_t input_thread;
pthread_t sound_thread;
void *input_thread_f(void *);
void *sound_thread_f(void *);


// Block ind updater==========
void block_ind_update(block *block_list, int camera_pos){
	int i;
	for (i = block_r; i < BLOCK_NUM; i++)
		if (block_list[i].hit.x > camera_pos + CAMERA_SIZE + LOAD_LIMIT)
			break;
	block_r = i;
	for (i = block_l; (i < block_r) && (i < BLOCK_NUM); i++){
		if (block_list[i].hit.x + block_list[i].hit.sx + 1 >= camera_pos + LOAD_LIMIT)
			break;
		block_list[i].loaded = 0;
	}
	block_l = i;
	for (i = block_l; i < block_r; i++)
		block_list[i].loaded = 1;
}

int main(){

	//Set up========================
	int err, col;
	static const char filename[] = "/dev/vga_ball";
	int ping_pong = 0;
	block_r = 0;
	block_l = 0;

	// Game OBJ
	mario_game game_0;

  	// r = libusb_init(&ctx);      // initialize a library session
  	// if (r < 0)
  	// {
    // 	printf("%s  %d\n", "Init Error", r); // there was an error
    // 	return 1;
  	// }
  	// libusb_set_debug(ctx, 3);                 // set verbosity level to 3, as suggested in the documentation
  	// cnt = libusb_get_device_list(ctx, &devs); // get the list of devices
  	// if (cnt < 0)
  	// {
    // 	printf("%s\n", "Get Device Error"); // there was an error
  	// }
  	// mouse = libusb_open_device_with_vid_pid(ctx, 0x0079, 0x0011);
	// if (mouse == NULL)
	// {
	// 	printf("%s\n", "Cannot open device");
	// 	libusb_free_device_list(devs, 1); // free the list, unref the devices in it
	// 	libusb_exit(ctx);                 // close the session
	// 	return 0;
	// }
	// else
	// {
	// 	printf("%s\n", "Device opened");
	// 	libusb_free_device_list(devs, 1); // free the list, unref the devices in it
	// 	if (libusb_kernel_driver_active(mouse, 0) == 1)
	// 	{ // find out if kernel driver is attached
	// 		printf("%s\n", "Kernel Driver Active");
	// 	  	if (libusb_detach_kernel_driver(mouse, 0) == 0) // detach it
	// 	    printf("%s\n", "Kernel Driver Detached!");
	// 	}
	// 	r = libusb_claim_interface(mouse, 0); // claim interface 0 (the first) of device (mine had just 1)
	// 	if (r < 0)
	// 	{
	// 	  	printf("%s\n", "Cannot Claim Interface");
	// 	  	return 1;
	// 	}
	// }

	// Open the USB keyboard
    if ((keyboard = openkeyboard(&endpoint_address)) == NULL) {
        fprintf(stderr, "Did not find a keyboard\n");
        exit(EXIT_FAILURE);
    }
	printf("%s\n", "Claimed Interface");

	// Open Avalon bus
	if ( (vga_ball_fd = open(filename, O_RDWR)) == -1) {
		fprintf(stderr, "could not open %s\n", filename);
		return -1;
  	}

	/////////////////////////////////////
	/* Start the network thread */
	pthread_create(&input_thread, NULL, input_thread_f, NULL);
	//pthread_create(&longpress_thread, NULL, longpress_thread_f, NULL);
	

	// Start a new game 
	new_game(&game_0);
	// loop to find all in-screen element first
	// ====================================
	while (1){

		//  new game if Mario is dead
		if (game_0.mario_0.mario_s == DEAD || current_key == KEY_NEWGAME) {new_game(&game_0); block_r = 0; block_l = 0;}
			
		// loop to include all the avaliable block / entities / mario
		block_ind_update(&(game_0.block_list[0]), game_0.camera_pos);
		// using LR ind / xy for entity
			//entity, tube, cloud, all included now
		
		//=================================
		
		// apply acceleration to all entity
		game_0.mario_0.acc_y = GRAVITY;
		if(game_0.mario_0.control_s == MARIO_NORMAL){
			if(game_0.mario_0.y_block == 1){
				switch (current_key){
					case KEY_LEFT:
						game_0.mario_0.acc_x = -WALK_ACC;
						game_0.mario_0.vis.flip = 1;
						break;
					case KEY_RIGHT:
						game_0.mario_0.acc_x = WALK_ACC;
						game_0.mario_0.vis.flip = 0;
						break;
					case KEY_JUMP:
						game_0.mario_0.hit.vy = (game_0.mario_0.mario_s == SMALL)? -JUMP_INIT_V_SMALL : -JUMP_INIT_V_LARGE;
						game_0.mario_0.acc_x = 0;
						game_0.mario_0.hit.vx *= 0.8;
						sound_ind = SOUND_JUMP;
						sound_new = 1;
						break;
					// Shut down
					default:
						game_0.mario_0.hit.vx = (fabs(game_0.mario_0.hit.vx) > 2 * SHUT_ACC)?
												(game_0.mario_0.hit.vx > 0)? game_0.mario_0.hit.vx - SHUT_ACC: game_0.mario_0.hit.vx + SHUT_ACC
												: 0;
						game_0.mario_0.acc_x = 0;
				}
			}
			//apply acceleration / volicy / input() jump to mario
			//base on the grounded / block x/ block y state from last frame
			game_0.mario_0.hit.vy = (game_0.mario_0.hit.vy + game_0.mario_0.acc_y > MAX_SPEED_V) ? MAX_SPEED_V :
									(game_0.mario_0.hit.vy + game_0.mario_0.acc_y < -MAX_SPEED_V_JUMP) ? -MAX_SPEED_V_JUMP: 
									game_0.mario_0.hit.vy + game_0.mario_0.acc_y;
			game_0.mario_0.hit.vx = (game_0.mario_0.hit.vx + game_0.mario_0.acc_x > MAX_SPEED_H) ? MAX_SPEED_H :
									(game_0.mario_0.hit.vx + game_0.mario_0.acc_x < -MAX_SPEED_H) ? -MAX_SPEED_H: 
									game_0.mario_0.hit.vx + game_0.mario_0.acc_x;
		}

		// Mush==============================

		for (int i = 0; i < MUSH_NUM; i++){
			if (game_0.mush_list[i].enable == 1 && game_0.mush_list[i].mush_s == MUSH_NORMAL){
					if (game_0.mush_list[i].hit.y > 500) game_0.mush_list[i].enable = 0;
					else{
						game_0.mush_list[i].acc_y = GRAVITY;

						if(game_0.mush_list[i].x_block != 0) game_0.mush_list[i].vis.extra_info = (game_0.mush_list[i].x_block == 1)? 
																							-1 : 1;
						// Fall?
						if (game_0.mush_list[i].y_block == 1) game_0.mush_list[i].hit.vx = MAX_SPEED_H * 0.5 * game_0.mush_list[i].vis.extra_info;
						else game_0.mush_list[i].hit.vx = 0;

						game_0.mush_list[i].hit.vy = (game_0.mush_list[i].hit.vy + game_0.mush_list[i].acc_y > MAX_SPEED_V) ? MAX_SPEED_V :
										game_0.mush_list[i].hit.vy + game_0.mush_list[i].acc_y; 
					}
			}
			
		}
		//================================

		// Goomba==============================

		for (int i = 0; i < GOOMBA_NUM; i++){
			if (game_0.goomba_list[i].enable == 1 && game_0.goomba_list[i].goomba_s == GOOMBA_NORMAL){
					if (game_0.goomba_list[i].hit.y > 500) game_0.goomba_list[i].enable = 0;
					else{
						game_0.goomba_list[i].acc_y = GRAVITY;

						if(game_0.goomba_list[i].x_block != 0 || game_0.goomba_list[i].hit.x > game_0.goomba_list[i].r_limit || game_0.goomba_list[i].hit.x < game_0.goomba_list[i].l_limit) game_0.goomba_list[i].vis.extra_info = (game_0.goomba_list[i].hit.x > game_0.goomba_list[i].r_limit || game_0.goomba_list[i].x_block == 1)? 
																							-1 : 1;
						// Fall?
						if (game_0.goomba_list[i].y_block == 1) game_0.goomba_list[i].hit.vx = MAX_SPEED_H * 0.5 * game_0.goomba_list[i].vis.extra_info;
						else game_0.goomba_list[i].hit.vx = 0;

						game_0.goomba_list[i].hit.vy = (game_0.goomba_list[i].hit.vy + game_0.goomba_list[i].acc_y > MAX_SPEED_V) ? MAX_SPEED_V :
										game_0.goomba_list[i].hit.vy + game_0.goomba_list[i].acc_y; 
					}
			}
			
		}
		//================================

		// Apply hit check to all entity
		// update state / hit / animation
		enum contact result;
		enum contact result2;
		// Coin
		for (int i = 0; i < COIN_NUM; i++){
			if(game_0.coin_list[i].enable == 1 && game_0.coin_list[i].hit.x >= game_0.camera_pos
			&& game_0.coin_list[i].hit.x <= game_0.camera_pos + CAMERA_SIZE + LOAD_LIMIT
			&& game_0.coin_list[i].coin_s == COIN_NORMAL){
				result = hitbox_contact(&(game_0.coin_list[i].hit), &(game_0.mario_0.hit));
				if (result != NONE){
					game_0.coin_list[i].coin_s = COIN_ANIMATE;
					game_0.coin_list[i].animate_frame_counter = frame_counter;
					sound_ind = SOUND_COIN;
					sound_new = 1;
				}
			}
		}

		// Mush
		for (int i = 0; i < MUSH_NUM; i++){
			if(game_0.mush_list[i].enable == 1 && game_0.mush_list[i].hit.x >= game_0.camera_pos
			&& game_0.mush_list[i].hit.x <= game_0.camera_pos + CAMERA_SIZE + LOAD_LIMIT
			&& game_0.mush_list[i].mush_s == MUSH_NORMAL){
				result = hitbox_contact(&(game_0.mush_list[i].hit), &(game_0.mario_0.hit));
				if (result != NONE){
					game_0.mush_list[i].enable = 0;
					game_0.mario_0.control_s = MARIO_ANIMATE;
					game_0.mario_0.animate_s = ENLARGE;
					game_0.mario_0.animate_frame_counter = frame_counter;
				}
			}
		}

		// Goomba
		for (int i = 0; i < GOOMBA_NUM; i++){
			if(game_0.goomba_list[i].enable == 1 && game_0.goomba_list[i].hit.x >= game_0.camera_pos
			&& game_0.goomba_list[i].hit.x <= game_0.camera_pos + CAMERA_SIZE + LOAD_LIMIT - 8
			&& game_0.goomba_list[i].goomba_s == GOOMBA_NORMAL){
				result = hitbox_contact(&(game_0.goomba_list[i].hit), &(game_0.mario_0.hit));
				result2 = hitbox_contact(&(game_0.mario_0.hit), &(game_0.goomba_list[i].hit));
				if (result != NONE || result2 != NONE){
					// Kill Goomba
					if (result == UP || result2 == DOWN){
						game_0.goomba_list[i].goomba_s = GOOMBA_ANIMATE;
						game_0.goomba_list[i].animate_frame_counter = frame_counter;
						// Jump again
						game_0.mario_0.hit.vy = (game_0.mario_0.mario_s == SMALL)? -JUMP_INIT_V_SMALL : -JUMP_INIT_V_LARGE;
					}
					// Damage to Mario
					else if (game_0.mario_0.game_s == MARIO_GAME_NORMAL){
						// Dead
						if (game_0.mario_0.mario_s == SMALL){
							game_0.mario_0.control_s = MARIO_ANIMATE;
							game_0.mario_0.animate_s = ANI_DEAD;
							game_0.mario_0.animate_frame_counter = frame_counter;
							sound_ind = SOUND_DEAD;
							sound_new = 1;
						}
						// Return to SMALL
						else{
							game_0.mario_0.control_s = MARIO_ANIMATE;
							game_0.mario_0.animate_s = HIT;
							game_0.mario_0.animate_frame_counter = frame_counter;
						}	
					}
				}
			}
		}
		
		//=================================

		// do the animation 


		//=======================
		//**** clear all the hit / block state first *******
		if(game_0.mario_0.control_s == MARIO_NORMAL){
			game_0.mario_0.x_block = 0; game_0.mario_0.y_block = 0;
			// apply hit check on the ground
			for (int i = 0; i < GROUND_NUM; i++){
				result = hitbox_contact(&(game_0.mario_0.hit), &(game_0.ground_list[i].hit));
				if (result == LEFT){
					game_0.mario_0.x_block = -1;
					game_0.mario_0.hit.vx = 0;
					game_0.mario_0.hit.x = game_0.ground_list[i].hit.x + game_0.ground_list[i].hit.sx;
				}
				else if (result == RIGHT){
					game_0.mario_0.x_block = 1;
					game_0.mario_0.hit.vx = 0;
					game_0.mario_0.hit.x = game_0.ground_list[i].hit.x - game_0.mario_0.hit.sx;
				}
				else if (result == UP){
					game_0.mario_0.y_block = -1;
					game_0.mario_0.hit.vy = 0;
					game_0.mario_0.hit.y = game_0.ground_list[i].hit.y + game_0.ground_list[i].hit.sy;
				}
				else if (result == DOWN){
					game_0.mario_0.y_block = 1;
					game_0.mario_0.hit.vy = 0;
					game_0.mario_0.hit.y = game_0.ground_list[i].hit.y - game_0.mario_0.hit.sy;
				}
			}

			// apply hit check on the Tube
			for (int i = 0; i < TUBE_NUM; i++){
				if (game_0.tube_list[i].enable == 1){
					result = hitbox_contact(&(game_0.mario_0.hit), &(game_0.tube_list[i].hit));
					if (result == LEFT){
						game_0.mario_0.x_block = -1;
						game_0.mario_0.hit.vx = 0;
						game_0.mario_0.hit.x = game_0.tube_list[i].hit.x + game_0.tube_list[i].hit.sx;
					}
					else if (result == RIGHT){
						game_0.mario_0.x_block = 1;
						game_0.mario_0.hit.vx = 0;
						game_0.mario_0.hit.x = game_0.tube_list[i].hit.x - game_0.mario_0.hit.sx;
					}
					else if (result == UP){
						game_0.mario_0.y_block = -1;
						game_0.mario_0.hit.vy = 0;
						game_0.mario_0.hit.y = game_0.tube_list[i].hit.y + game_0.tube_list[i].hit.sy;
					}
					else if (result == DOWN){
						game_0.mario_0.y_block = 1;
						game_0.mario_0.hit.vy = 0;
						game_0.mario_0.hit.y = game_0.tube_list[i].hit.y - game_0.mario_0.hit.sy;
					}
				}
			}

			//Hit the block
			for(int i = block_l; i < block_r; i++){
				if (game_0.block_list[i].enable == 1){
					// NEED TO CHECK THE SIZE *********
					if (game_0.mario_0.hit.sy > game_0.block_list[i].hit.sy){
						result = hitbox_contact(&(game_0.block_list[i].hit), &(game_0.mario_0.hit));
						switch (result){
							case LEFT: result = RIGHT; break;
							case RIGHT: result = LEFT; break;
							default: result = hitbox_contact(&(game_0.mario_0.hit), &(game_0.block_list[i].hit));
						}
					}
					else result = hitbox_contact(&(game_0.mario_0.hit), &(game_0.block_list[i].hit));

					if (result == LEFT && game_0.block_list[i].R == 0){
						game_0.mario_0.x_block = -1;
						game_0.mario_0.hit.vx = 0;
						game_0.mario_0.hit.x = game_0.block_list[i].hit.x + game_0.block_list[i].hit.sx;
					}
					else if (result == RIGHT && game_0.block_list[i].L == 0){
						game_0.mario_0.x_block = 1;
						game_0.mario_0.hit.vx = 0;
						game_0.mario_0.hit.x = game_0.block_list[i].hit.x - game_0.mario_0.hit.sx;
					}
					// Head on the Block, do animation
					else if (result == UP  && game_0.block_list[i].D == 0){
						game_0.mario_0.y_block = -1;
						game_0.mario_0.hit.vy = game_0.mario_0.hit.vy * -0.5;
						game_0.mario_0.hit.y = game_0.block_list[i].hit.y + game_0.block_list[i].hit.sy;
						// Breaking the block ******* if mario game_s is LARGE
						if (game_0.block_list[i].block_t == TYPE_A && game_0.block_list[i].block_s == BLOCK_NORMAL && game_0.mario_0.mario_s == LARGE){
							for(int j = block_l; j < block_r; j++){
								if (game_0.block_list[j].enable == 1){
									// For block on LEFT, enable R
									if (game_0.block_list[j].hit.x == game_0.block_list[i].hit.x - 16) game_0.block_list[j].R = 0;
									// For block on RIGHT, enable L
									else if (game_0.block_list[j].hit.x == game_0.block_list[i].hit.x + 16) game_0.block_list[j].L = 0;	
									// For block on UP, enable D
									else if (game_0.block_list[j].hit.y == game_0.block_list[i].hit.y - 16) game_0.block_list[j].D = 0;		
								}	
							}
							game_0.block_list[i].block_s = BLOCK_ANIMATE;
							game_0.block_list[i].animate_frame_counter = frame_counter;
							sound_ind = SOUND_BLOCK;
							sound_new = 1;		
						}
						// Hitting an coin block
						if (game_0.block_list[i].block_t == OBJ_C && game_0.block_list[i].block_s == BLOCK_NORMAL){
							game_0.block_list[i].block_s = BLOCK_ANIMATE;
							game_0.block_list[i].animate_frame_counter = frame_counter;	
							// generate an extra item
							game_0.coin_list[COIN_NUM-1].hit.x = game_0.block_list[i].hit.x + 4;
							game_0.coin_list[COIN_NUM-1].hit.y = game_0.block_list[i].hit.y;
							game_0.coin_list[COIN_NUM-1].enable = 1;
							game_0.coin_list[COIN_NUM-1].loaded = 1;
							game_0.coin_list[COIN_NUM-1].coin_s = COIN_ANIMATE;
							game_0.coin_list[COIN_NUM-1].animate_frame_counter = frame_counter;	
						}
						// Hitting an mush block
						if (game_0.block_list[i].block_t == OBJ_M && game_0.block_list[i].block_s == BLOCK_NORMAL){
							game_0.block_list[i].block_s = BLOCK_ANIMATE;
							game_0.block_list[i].animate_frame_counter = frame_counter;	
							// generate an extra item
							game_0.mush_list[MUSH_NUM-1].hit.x = game_0.block_list[i].hit.x;
							game_0.mush_list[MUSH_NUM-1].hit.y = game_0.block_list[i].hit.y;
							game_0.mush_list[MUSH_NUM-1].enable = 1;
							game_0.mush_list[MUSH_NUM-1].loaded = 1;
							game_0.mush_list[MUSH_NUM-1].mush_s = MUSH_ANIMATE;
							game_0.mush_list[MUSH_NUM-1].animate_frame_counter = frame_counter;	
							game_0.mush_list[MUSH_NUM-1].vis.extra_info = 1;
						}
					}
					else if (result == DOWN && game_0.block_list[i].U == 0){
						game_0.mario_0.y_block = 1;
						game_0.mario_0.hit.vy = 0;
						game_0.mario_0.hit.y = game_0.block_list[i].hit.y - game_0.mario_0.hit.sy;
					}
				}
			}
			// update position
			game_0.mario_0.hit.x += game_0.mario_0.hit.vx; game_0.mario_0.hit.y += game_0.mario_0.hit.vy;
			// On the edge:
			if (game_0.mario_0.hit.x <= game_0.camera_pos + LOAD_LIMIT) {
				game_0.mario_0.hit.x = game_0.camera_pos + LOAD_LIMIT;
				game_0.mario_0.hit.vx = (game_0.mario_0.hit.vx < 0)? 0 : game_0.mario_0.hit.vx;
				}
			if (game_0.mario_0.hit.y >= 384) {
				game_0.mario_0.control_s = MARIO_ANIMATE;
				game_0.mario_0.animate_s = ANI_DEAD;
				game_0.mario_0.animate_frame_counter = frame_counter;
				sound_ind = SOUND_DEAD;
				sound_new = 1;
			}
		}

		// MUSH ================================
		for (int j = 0; j < MUSH_NUM; j++){
			if (game_0.mush_list[j].enable == 1 && game_0.mush_list[j].mush_s == MUSH_NORMAL){
				game_0.mush_list[j].x_block = 0; game_0.mush_list[j].y_block = 0;
				for (int i = 0; i < GROUND_NUM; i++){
					result = hitbox_contact(&(game_0.mush_list[j].hit), &(game_0.ground_list[i].hit));
					if (result == LEFT){
						game_0.mush_list[j].x_block = -1;
						game_0.mush_list[j].hit.vx = 0;
						game_0.mush_list[j].hit.x = game_0.ground_list[i].hit.x + game_0.ground_list[i].hit.sx;
					}
					else if (result == RIGHT){
						game_0.mush_list[j].x_block = 1;
						game_0.mush_list[j].hit.vx = 0;
						game_0.mush_list[j].hit.x = game_0.ground_list[i].hit.x - game_0.mush_list[j].hit.sx;
					}
					else if (result == DOWN){
						game_0.mush_list[j].y_block = 1;
						game_0.mush_list[j].hit.vy = 0;
						game_0.mush_list[j].hit.y = game_0.ground_list[i].hit.y - game_0.mush_list[j].hit.sy;
					}			
				}

				for (int i = 0; i < TUBE_NUM; i++){
					if (game_0.tube_list[i].enable == 1){
						result = hitbox_contact(&(game_0.mush_list[j].hit), &(game_0.tube_list[i].hit));
						if (result == LEFT){
							game_0.mush_list[j].x_block = -1;
							game_0.mush_list[j].hit.vx = 0;
							game_0.mush_list[j].hit.x = game_0.tube_list[i].hit.x + game_0.tube_list[i].hit.sx;
						}
						else if (result == RIGHT){
							game_0.mush_list[j].x_block = 1;
							game_0.mush_list[j].hit.vx = 0;
							game_0.mush_list[j].hit.x = game_0.tube_list[i].hit.x - game_0.mush_list[j].hit.sx;
						}
						else if (result == DOWN){
							game_0.mush_list[j].y_block = 1;
							game_0.mush_list[j].hit.vy = 0;
							game_0.mush_list[j].hit.y = game_0.tube_list[i].hit.y - game_0.mush_list[j].hit.sy;
						}
					}			
				}				
					
				for(int i = block_l; i < block_r; i++){
					if (game_0.block_list[i].enable == 1){
						result = hitbox_contact(&(game_0.mush_list[j].hit), &(game_0.block_list[i].hit));
						if (result == LEFT && game_0.block_list[i].R == 0){
							game_0.mush_list[j].x_block = -1;
							game_0.mush_list[j].hit.vx = 0;
							game_0.mush_list[j].hit.x = game_0.block_list[i].hit.x + game_0.block_list[i].hit.sx;
						}
						else if (result == RIGHT && game_0.block_list[i].L == 0){
							game_0.mush_list[j].x_block = 1;
							game_0.mush_list[j].hit.vx = 0;
							game_0.mush_list[j].hit.x = game_0.block_list[i].hit.x - game_0.mush_list[j].hit.sx;
						}
						else if (result == DOWN && game_0.block_list[i].U == 0){
							game_0.mush_list[j].y_block = 1;
							game_0.mush_list[j].hit.vy = 0;
							game_0.mush_list[j].hit.y = game_0.block_list[i].hit.y - game_0.mush_list[j].hit.sy;
						}
					}
				}
				// update position
				game_0.mush_list[j].hit.x += game_0.mush_list[j].hit.vx; game_0.mush_list[j].hit.y += game_0.mush_list[j].hit.vy;			
			}
		}
		// GOOMBA=======================================================
		for (int j = 0; j < GOOMBA_NUM; j++){
			if (game_0.goomba_list[j].enable == 1 && game_0.goomba_list[j].goomba_s == GOOMBA_NORMAL && 
				game_0.goomba_list[j].hit.x >= game_0.camera_pos &&
				game_0.goomba_list[j].hit.x <= game_0.camera_pos + CAMERA_SIZE + LOAD_LIMIT){
				game_0.goomba_list[j].x_block = 0; game_0.goomba_list[j].y_block = 0;
				for (int i = 0; i < GROUND_NUM; i++){
					result = hitbox_contact(&(game_0.goomba_list[j].hit), &(game_0.ground_list[i].hit));
					if (result == LEFT){
						game_0.goomba_list[j].x_block = -1;
						game_0.goomba_list[j].hit.vx = 0;
						game_0.goomba_list[j].hit.x = game_0.ground_list[i].hit.x + game_0.ground_list[i].hit.sx;
					}
					else if (result == RIGHT){
						game_0.goomba_list[j].x_block = 1;
						game_0.goomba_list[j].hit.vx = 0;
						game_0.goomba_list[j].hit.x = game_0.ground_list[i].hit.x - game_0.goomba_list[j].hit.sx;
					}
					else if (result == DOWN){
						game_0.goomba_list[j].y_block = 1;
						game_0.goomba_list[j].hit.vy = 0;
						game_0.goomba_list[j].hit.y = game_0.ground_list[i].hit.y - game_0.goomba_list[j].hit.sy;
					}			
				}

				for (int i = 0; i < TUBE_NUM; i++){
					if (game_0.tube_list[i].enable == 1){
						result = hitbox_contact(&(game_0.goomba_list[j].hit), &(game_0.tube_list[i].hit));
						if (result == LEFT){
							game_0.goomba_list[j].x_block = -1;
							game_0.goomba_list[j].hit.vx = 0;
							game_0.goomba_list[j].hit.x = game_0.tube_list[i].hit.x + game_0.tube_list[i].hit.sx;
						}
						else if (result == RIGHT){
							game_0.goomba_list[j].x_block = 1;
							game_0.goomba_list[j].hit.vx = 0;
							game_0.goomba_list[j].hit.x = game_0.tube_list[i].hit.x - game_0.goomba_list[j].hit.sx;
						}
						else if (result == DOWN){
							game_0.goomba_list[j].y_block = 1;
							game_0.goomba_list[j].hit.vy = 0;
							game_0.goomba_list[j].hit.y = game_0.tube_list[i].hit.y - game_0.goomba_list[j].hit.sy;
						}		
					}	
				}

				for(int i = block_l; i < block_r; i++){
					if (game_0.block_list[i].enable == 1){
						// NEED TO CHECK THE SIZE *********
						if (game_0.goomba_list[j].hit.sy > game_0.block_list[i].hit.sy){
							result = hitbox_contact(&(game_0.block_list[i].hit), &(game_0.goomba_list[j].hit));
							switch (result){
								case LEFT: result = RIGHT; break;
								case RIGHT: result = LEFT; break;
								default: result = hitbox_contact(&(game_0.goomba_list[j].hit), &(game_0.block_list[i].hit));
							}
						}
						else result = hitbox_contact(&(game_0.goomba_list[j].hit), &(game_0.block_list[i].hit));
						if (result == LEFT && game_0.block_list[i].R == 0){
							game_0.goomba_list[j].x_block = -1;
							game_0.goomba_list[j].hit.vx = 0;
							game_0.goomba_list[j].hit.x = game_0.block_list[i].hit.x + game_0.block_list[i].hit.sx;
						}
						else if (result == RIGHT && game_0.block_list[i].L == 0){
							game_0.goomba_list[j].x_block = 1;
							game_0.goomba_list[j].hit.vx = 0;
							game_0.goomba_list[j].hit.x = game_0.block_list[i].hit.x - game_0.goomba_list[j].hit.sx;
						}
						else if (result == DOWN && game_0.block_list[i].U == 0){
							game_0.goomba_list[j].y_block = 1;
							game_0.goomba_list[j].hit.vy = 0;
							game_0.goomba_list[j].hit.y = game_0.block_list[i].hit.y - game_0.goomba_list[j].hit.sy;
						}
					}
				}
				// update position
				game_0.goomba_list[j].hit.x += game_0.goomba_list[j].hit.vx; game_0.goomba_list[j].hit.y += game_0.goomba_list[j].hit.vy;			
			}
		}

		// update block x block y state (and update x, y position)
		// modify the velocity of all entity
		// update position 
		//=============================
		// update the camera position
		if(((int)game_0.mario_0.hit.x) > game_0.camera_pos + CAMERA_SIZE/2 + LOAD_LIMIT){
			game_0.camera_pos = (((int)game_0.mario_0.hit.x) - CAMERA_SIZE/2 - LOAD_LIMIT > 0)?
								((int)game_0.mario_0.hit.x) - CAMERA_SIZE/2 - LOAD_LIMIT : 0;
		}
		// generate all the ppu info
		// flush the frame into ppu
		//test=======================
		flush_ping_pong_frame(&game_0, ping_pong);
		ping_pong = (ping_pong == 0)? 1 : 0;
		usleep(16667);
	}

	pthread_cancel(input_thread);
	pthread_join(input_thread, NULL);

	return 0;
}

void *input_thread_f(void *ignored)
{
    struct usb_keyboard_packet packet;
    int transferred;
    int r;
    struct timeval timeout = { 0, 500000 }; // 500 ms timeout
	uint8_t first, second, chosen; 

    for (;;) {
        r = libusb_interrupt_transfer(keyboard, endpoint_address, (unsigned char *)&packet, sizeof(packet), &transferred, 0);
        if (r == 0 && transferred == sizeof(packet)) {

			first = packet.keycode[0]; 
			second = packet.keycode[1];
			chosen = 0;

			if (first != 0 && second != 0) {

				if (first == second) {
					usleep(5000);
					continue; 
				} else {
					chosen = second; 
				}

			} else {
				chosen = first; 
			}

            switch(chosen) {  // Check the first keycode in the array
                case 0x2C: // Space bar
                    current_key = KEY_JUMP;
                    break;
                case 0x04: // 'a' key
                    current_key = KEY_LEFT;
                    break;
                case 0x07: // 'd' key
                    current_key = KEY_RIGHT;
                    break;
                case 0x0A: // 'g' key
                    current_key = KEY_NEWGAME;
                    break;
                default:
                    current_key = KEY_NONE;
                    break;
            }
        } else {
            if (r == LIBUSB_ERROR_NO_DEVICE) {
                // Handle device disconnection
                fprintf(stderr, "Keyboard disconnected.\n");
                break; // Exit or attempt to reconnect
            }
            // Handle other errors or no data transferred
            fprintf(stderr, "Transfer error: %s\n", libusb_error_name(r));
            current_key = KEY_NONE;
            libusb_handle_events_timeout(NULL, &timeout);
        }
    }
    return NULL;
}



