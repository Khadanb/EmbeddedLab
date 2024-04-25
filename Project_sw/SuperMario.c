/*
 * Game Logic
 * Edited on April 22 2024 by Brandon Khadan
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
#define SOUND_JUMP 0
#define SOUND_COIN 0
#define SOUND_BLOCK 0
#define SOUND_DEAD 0
#define SOUND_NONE 5

int vga_ball_fd;

int block_index;
int coin_index;

int info_001 = 1;
int info_010 = 2;
int info_011 = 3;
int info_100 = 4;

int block_r = 0;
int block_l = 0;
int frame_counter = 0;

int sound_new = 0;
int sound_ind = 0;

struct libusb_device_handle *keyboard;
enum key_input{KEY_NONE, KEY_JUMP, KEY_LEFT, KEY_RIGHT, KEY_NEWGAME, KEY_END};
enum key_input current_key;
uint8_t endpoint_address;
pthread_t input_thread;

void write_to_hardware(int vga_fd, int register_address, int data) {
	vga_ball_arg_t vla;
	vla.addr = register_address;
	vla.info = data;


	if (ioctl(vga_fd, VGA_BALL_WRITE_BACKGROUND, &vla) < 0) {
		fprintf(stderr, "Failed to write data to hardware\n");
	}
}

void flush_mario(const Entity *entity, int frame_select) {

    int visible = entity->render.visible;
    int flip = entity->render.flip;
    int x = entity->position.x;
    int y = entity->position.y;
    int pattern_code = entity->render.pattern_code;

	//printf("Flushing MARIO - Visible: %d, Flip: %d, X: %d, Y: %d, Pattern: %d\n", visible, flip, x, y, pattern_code);

    write_to_hardware(vga_ball_fd, 0, (int)((1 << 26) + (1 << 17) + (info_001 << 14) + (frame_select << 13) + (1 << 12) + (flip << 11) + (pattern_code & 0x1F)));
    write_to_hardware(vga_ball_fd, 0, (int)((1 << 26) + (1 << 17) + (info_010 << 14) + (frame_select << 13) + (x & 0x3FF)));
    write_to_hardware(vga_ball_fd, 0, (int)((1 << 26) + (1 << 17) + (info_011 << 14) + (frame_select << 13) + (y & 0x3FF)));
}

void flush_mush(const Entity *entity, int frame_select) {

    int visible = entity->render.visible;
    int flip = entity->render.flip;
    int x = entity->position.x;
    int y = entity->position.y;
    int pattern_code = entity->render.pattern_code;

    //printf("Flushing MUSHROOM - Visible: %d, Flip: %d, X: %d, Y: %d, Pattern: %d\n", visible, flip, x, y, pattern_code);

    write_to_hardware(vga_ball_fd, 0, (int)((9 << 26) + ((0&0x1F) << 21) + (1 << 17) + (info_001 << 14) + (frame_select << 13) + (1 << 12) + (0 << 11) + (pattern_code & 0x1F)));
    write_to_hardware(vga_ball_fd, 0, (int)((9 << 26) + ((0&0x1F) << 21) + (1 << 17) + (info_010 << 14) + (frame_select << 13) + (x & 0x3FF)));
    write_to_hardware(vga_ball_fd, 0, (int)((9 << 26) + ((0&0x1F) << 21) + (1 << 17) + (info_011 << 14) + (frame_select << 13) + (y & 0x3FF)));
}

void flush_goomba(const Entity *entity, int frame_select) {

    int visible = entity->render.visible;
    int flip = entity->render.flip;
    int x = entity->position.x;
    int y = entity->position.y;
    int pattern_code = entity->render.pattern_code;

    //printf("Flushing GOOMBA - Visible: %d, Flip: %d, X: %d, Y: %d, Pattern: %d\n", visible, flip, x, y, pattern_code);

    write_to_hardware(vga_ball_fd, 0, (int)((5 << 26) + (1 << 17) + (info_001 << 14) + (frame_select << 13) + (1 << 12) + (flip << 11) + (pattern_code & 0x1F)));
    write_to_hardware(vga_ball_fd, 0, (int)((5 << 26) + (1 << 17) + (info_010 << 14) + (frame_select << 13) + (x & 0x3FF)));
    write_to_hardware(vga_ball_fd, 0, (int)((5 << 26) + (1 << 17) + (info_011 << 14) + (frame_select << 13) + (y & 0x3FF)));
}

void flush_coin(const Entity *entity, int frame_select) {

    int visible = entity->render.visible;
    int flip = entity->render.flip;
    int x = entity->position.x;
    int y = entity->position.y;
    int pattern_code = entity->render.pattern_code;
    int entityTypeCode = entity->state.type; 

    //printf("Flushing COIN - Type: %d, Visible: %d, Flip: %d, X: %d, Y: %d, Pattern: %d\n", entityTypeCode, visible, flip, x, y, pattern_code);

    write_to_hardware(vga_ball_fd, 0, (int)((3 << 26) + ((coin_index & 0x1F) << 21) + (1 << 17) + (info_001 << 14) + (frame_select << 13) + (1 << 12) + (0 << 11) + (pattern_code & 0x1F)));
    write_to_hardware(vga_ball_fd, 0, (int)((3 << 26) + ((coin_index & 0x1F) << 21) + (1 << 17) + (info_010 << 14) + (frame_select << 13) + (x & 0x3FF)));
    write_to_hardware(vga_ball_fd, 0, (int)((3 << 26) + ((coin_index & 0x1F) << 21) + (1 << 17) + (info_011 << 14) + (frame_select << 13) + (y & 0x3FF)));
	coin_index += 1; 
}


void flush_block(const Entity *entity, int frame_select) {

    int visible = entity->render.visible;
    int flip = entity->render.flip;
    int x = entity->position.x;
    int y = entity->position.y;
    int pattern_code = entity->render.pattern_code;
    int entityTypeCode = entity->state.type; 

    //printf("Flushing BLOCK - Type: %d, Visible: %d, Flip: %d, X: %d, Y: %d, Pattern: %d\n", entityTypeCode, visible, flip, x, y, pattern_code);

    write_to_hardware(vga_ball_fd, 0, (int)((2 << 26) + ((block_index & 0x1F) << 21) + (1 << 17) + (info_001 << 14) + (frame_select << 13) + (1 << 12) + (0 << 11) + (pattern_code & 0x1F)));
    write_to_hardware(vga_ball_fd, 0, (int)((2 << 26) + ((block_index & 0x1F) << 21) + (1 << 17) + (info_010 << 14) + (frame_select << 13) + (x & 0x3FF)));
    write_to_hardware(vga_ball_fd, 0, (int)((2 << 26) + ((block_index & 0x1F) << 21) + (1 << 17) + (info_011 << 14) + (frame_select << 13) + (y & 0x3FF)));
	block_index += 1;
}

void flush_tube(const Entity *entity, int frame_select) {
    int visible = entity->render.visible;
    int flip = entity->render.flip;
    int x = entity->position.x;
    int y = entity->position.y;
    int pattern_code = entity->render.pattern_code;

    //printf("Flushing TUBE - Visible: %d, Flip: %d, X: %d, Y: %d, Pattern: %d\n", visible, flip, x, y, pattern_code);

    write_to_hardware(vga_ball_fd, 0, (int)((10 << 26) + ((0&0x1F) << 21) + (1 << 17) + (info_001 << 14) + (frame_select << 13) + (1 << 12) + (flip << 11) + (ANI_TUBE_H & 0x1F)));
    write_to_hardware(vga_ball_fd, 0, (int)((10 << 26) + ((0&0x1F) << 21) + (1 << 17) + (info_010 << 14) + (frame_select << 13) + (x & 0x3FF)));
    write_to_hardware(vga_ball_fd, 0, (int)((10 << 26) + ((0&0x1F) << 21) + (1 << 17) + (info_011 << 14) + (frame_select << 13) + (y & 0x3FF)));

	write_to_hardware(vga_ball_fd, 0, (int)((10 << 26) + ((1&0x1F) << 21) + (1 << 17) + (info_001 << 14) + (frame_select << 13) + (1 << 12) + (flip << 11) + (ANI_TUBE_B & 0x1F)));
	write_to_hardware(vga_ball_fd, 0, (int)((10 << 26) + ((1&0x1F) << 21) + (1 << 17) + (info_010 << 14) + (frame_select << 13) + (x & 0x3FF)));
	write_to_hardware(vga_ball_fd, 0, (int)((10 << 26) + ((1&0x1F) << 21) + (1 << 17) + (info_011 << 14) + (frame_select << 13) + (y & 0x3FF)));
}

void flush_cloud(const Entity *entity, int frame_select) {

    int visible = entity->render.visible;
    int flip = entity->render.flip;
    int x = entity->position.x;
    int y = entity->position.y;
    int pattern_code = entity->render.pattern_code;

    //printf("Flushing CLOUD - Visible: %d, Flip: %d, X: %d, Y: %d, Pattern: %d\n", visible, flip, x, y, pattern_code);

    write_to_hardware(vga_ball_fd, 0, (int)((14 << 26) + ((0&0x1F) << 21) + (1 << 17) + (info_001 << 14) + (frame_select << 13) + (1 << 12) + (flip << 11) + (pattern_code & 0x1F)));
    write_to_hardware(vga_ball_fd, 0, (int)((14 << 26) + ((0&0x1F) << 21) + (1 << 17) + (info_010 << 14) + (frame_select << 13) + (x & 0x3FF)));
    write_to_hardware(vga_ball_fd, 0, (int)((14 << 26) + ((0&0x1F) << 21) + (1 << 17) + (info_011 << 14) + (frame_select << 13) + (y & 0x3FF)));

}

void flush_entity(const Entity *entity, int frame_select, int camera_pos) {
    if (entity->render.pattern_code > 6 || entity->render.pattern_code < 0 ) return; 
    if (entity->state.type > TYPE_EMP || entity->state.type < 0) return;

    switch (entity->state.type) {
        case TYPE_MARIO_SMALL:
        case TYPE_MARIO_LARGE:
            flush_mario(entity, frame_select);
            break;
        case TYPE_MUSHROOM:
            flush_mush(entity, frame_select);
            break;
        case TYPE_GOOMBA:
            flush_goomba(entity, frame_select);
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
            flush_block(entity, frame_select);
            break;
        case TYPE_COIN:
            flush_coin(entity, frame_select);
            break;
        case TYPE_TUBE:
            flush_tube(entity, frame_select);
            break;
        case TYPE_CLOUD:
            flush_cloud(entity, frame_select);
            break;
        default:
			//printf("ERROR: Unkown Entity Found!\n");
            break;
    }
}

void flush_ground(int camera_pos, int frame_select) {
    // Arrays for left side and width of ground segments
    int ground_l[] = {GROUND_0_L, GROUND_1_L, GROUND_2_L, GROUND_3_L};
    int ground_w[] = {GROUND_0_W, GROUND_1_W, GROUND_2_W, GROUND_3_W};

    for (int i = 0; i < sizeof(ground_l)/sizeof(ground_l[0]); i++) {
        // Calculate the right side of the ground segment
        int ground_r = ground_l[i] + ground_w[i];

        // Determine the visible segment based on the camera position and screen size
        int visible_left = (ground_l[i] >= camera_pos) ? ground_l[i] - camera_pos : 0;
        int visible_right = (ground_r - camera_pos <= CAMERA_SIZE + LOAD_LIMIT) ? ground_r - camera_pos : CAMERA_SIZE + LOAD_LIMIT;

        // Check if there is a visible section
        if (visible_right > visible_left) {
            // Write left side of the ground segment
            write_to_hardware(vga_ball_fd, 0, (int)((15 << 26) + ((0&0x1F) << 21) + (1 << 17) + (info_001 << 14) + (frame_select << 13) + (1 << 12) + (0 << 11) + (0 & 0x1F)));
            write_to_hardware(vga_ball_fd, 0, (int)((15 << 26) + ((0&0x1F) << 21) + (1 << 17) + (info_010 << 14) + (frame_select << 13) + ((15 - (camera_pos%16)) & 0x3FF)));
            write_to_hardware(vga_ball_fd, 0, (int)((15 << 26) + ((0&0x1F) << 21) + (1 << 17) + (info_011 << 14) + (frame_select << 13) + (visible_left & 0x3FF)));
			write_to_hardware(vga_ball_fd, 0, (int)((15 << 26) + ((0&0x1F) << 21) + (1 << 17) + (info_100 << 14) + (frame_select << 13) + (visible_right & 0x3FF)));
        }
    }
}


void flush_frame(Game *game, int frame_select) {
	int entity_index;
	block_index = 0;
	coin_index = 0;

	for (entity_index = 0; entity_index < MAX_ENTITIES; entity_index++) {
		Entity *entity = &game->entities[entity_index];

		if (entity->state.active && entity->render.visible) {

			if (entity->position.x >= game->camera_pos &&
				entity->position.x <= game->camera_pos + CAMERA_SIZE + LOAD_LIMIT) {

				flush_entity(entity, frame_select, game->camera_pos);
			}
		}
	}

	write_to_hardware(vga_ball_fd, 0, (int)((1 << 26) + (0xf << 17) + (frame_select << 13)));

	frame_counter = (frame_counter >= FRAME_LIMIT) ? 0 : frame_counter + 1;

	if (sound_new == 1) {
		write_to_hardware(vga_ball_fd, 4, (int)(SOUND_NONE));
		sound_new = 0;
	} else {
		write_to_hardware(vga_ball_fd, 4, (int)(sound_ind));
	}
}


void entity_activation_update(Game *game, int camera_pos){
	int entity_index;
	for (entity_index = 0; entity_index < MAX_ENTITIES; entity_index++) {
		Entity *entity = &game->entities[entity_index];

		if (entity->position.x > camera_pos + CAMERA_SIZE + LOAD_LIMIT) {

			entity->state.active = 0;
		} else if (entity->position.x + entity->position.width + 1 < camera_pos - LOAD_LIMIT) {

			entity->state.active = 0;
		} else {

			entity->state.active = 1;
		}
	}
}

void *input_thread_function(void *ignored)
{
	struct usb_keyboard_packet packet;
	int transferred;
	int r;
	struct timeval timeout = { 0, 500000 };
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

			switch(chosen) {
				case 0x2C:
					current_key = KEY_JUMP;
					break;
				case 0x04:
					current_key = KEY_LEFT;
					break;
				case 0x07:
					current_key = KEY_RIGHT;
					break;
				case 0x0A:
					current_key = KEY_NEWGAME;
					break;
				default:
					current_key = KEY_NONE;
					break;
			}
		} else {
			if (r == LIBUSB_ERROR_NO_DEVICE) {

				fprintf(stderr, "Keyboard disconnected.\n");
				break;
			}

			fprintf(stderr, "Transfer error: %s\n", libusb_error_name(r));
			current_key = KEY_NONE;
			libusb_handle_events_timeout(NULL, &timeout);
		}
	}
	return NULL;
}

void handle_collision_with_mushroom(Entity *mario, Entity *other, enum contact type) {
	mario->state.state = STATE_ENLARGE;
	other->state.active = 0;
}

void handle_collision_with_goomba(Entity *mario, Entity *other, enum contact type) {
	if (type == UP) {
		other->state.state = STATE_DEAD;
		mario->motion.vy = -JUMP_INIT_V_SMALL;
	} else {

		mario->state.state = (mario->state.type == TYPE_MARIO_SMALL) ? STATE_DEAD : STATE_HIT;
	}
}

void handle_collision_with_coin(Entity *mario, Entity *other, enum contact type) {

	other->state.active = 0;
}

void handle_collision_with_block(Entity *mario, Entity *other, enum contact type) {
	if (type == UP) {
		other->state.state = BLOCK_ANIMATE;
		mario->motion.vy = 0;
	}
}

void handle_collision_with_tube(Entity *mario, Entity *other, enum contact type) {
	if (type == LEFT || type == RIGHT) {
		mario->motion.vx = 0;
	} else if (type == UP) {
		mario->motion.vy = 0;
	}
}

void handle_collision_with_ground(Entity *mario, Entity *other, enum contact type) {
	if (type == DOWN) {
		mario->motion.vy = 0;
		mario->position.y = other->position.y - mario->position.height;
	}
}

void process_mario_logic(Entity *mario, Game *game) {

	if (mario == NULL) {
		fprintf(stderr, "Mario entity is NULL\n");
		return;
	}

	mario->motion.ay = GRAVITY;

	if (current_key == KEY_LEFT) {
		mario->motion.ax = -WALK_ACC;
		mario->render.flip = 1;
	} else if (current_key == KEY_RIGHT) {
		mario->motion.ax = WALK_ACC;
		mario->render.flip = 0;
	}

	if (current_key == KEY_JUMP) {
		mario->motion.vy = -JUMP_INIT_V_SMALL;
	}

	mario->motion.vx += mario->motion.ax;
	mario->motion.vy += mario->motion.ay;

	mario->motion.vx = fminf(fmaxf(mario->motion.vx, -MAX_SPEED_H), MAX_SPEED_H);
	mario->motion.vy = fminf(fmaxf(mario->motion.vy, -MAX_SPEED_V_JUMP), MAX_SPEED_V);

	mario->position.x += mario->motion.vx;
	mario->position.y += mario->motion.vy;


	for (int i = 1; i < MAX_ENTITIES; i++) {
		Entity *other = &game->entities[i];

		if (other == NULL || !other->state.active) continue;

		enum contact contactType = hitbox_contact(mario, other);
		if (contactType != NONE) {
			switch (other->state.type) {
				case TYPE_MUSHROOM:
					handle_collision_with_mushroom(mario, other, contactType);
					break;
				case TYPE_GOOMBA:
					handle_collision_with_goomba(mario, other, contactType);
					break;
				case TYPE_COIN:
					handle_collision_with_coin(mario, other, contactType);
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
					handle_collision_with_block(mario, other, contactType);
					break;
				case TYPE_TUBE:
					handle_collision_with_tube(mario, other, contactType);
					break;
				case TYPE_GROUND:
					handle_collision_with_ground(mario, other, contactType);
					break;
				case TYPE_CLOUD:
					break;
				default:
					break;
			}
		}
	}

	// mario->motion.ax = -0.1;

	if (mario->position.y > GROUND_LEVEL) {
		mario->position.y = GROUND_LEVEL;
		mario->motion.vy = 0;
	}
}

void process_mushroom_logic(Entity *mushroom, Game *game) {
	if (!mushroom->state.active)
		return;

	mushroom->motion.ay = GRAVITY;
	mushroom->motion.vx = (mushroom->render.flip == 0) ? MAX_SPEED_H * 0.5 : -MAX_SPEED_H * 0.5;

	for (int i = 0; i < MAX_ENTITIES; i++) {
		Entity *other = &game->entities[i];
		if (other == NULL || !other->state.active) continue;

		enum contact contactType = hitbox_contact(mushroom, other);
		if (contactType != NONE) {
			switch (other->state.type) {
				case TYPE_MARIO_SMALL:
				case TYPE_MARIO_LARGE:
					mushroom->state.active = 0;
					break;
				case TYPE_GROUND:
					if (contactType == DOWN) {
						mushroom->motion.vy = 0;
						mushroom->position.y = other->position.y - mushroom->position.height;
					}
					break;
				case TYPE_TUBE:
				case TYPE_BLOCK_A:
				case TYPE_BLOCK_B_1:
				case TYPE_BLOCK_B_2:
				case TYPE_BLOCK_B_3:
				case TYPE_BLOCK_B_4:
				case TYPE_BLOCK_B_16:
				case TYPE_BLOCK_A_H_8:
				case TYPE_BLOCK_OBJ_C:
				case TYPE_BLOCK_OBJ_M:
					if (contactType == LEFT || contactType == RIGHT) {
						mushroom->render.flip = (mushroom->render.flip == 0) ? 1 : 0;
						mushroom->motion.vx = -mushroom->motion.vx;
					}
					break;
			}
		}
	}

	mushroom->motion.vy += mushroom->motion.ay;
	mushroom->position.x += mushroom->motion.vx;
	mushroom->position.y += mushroom->motion.vy;
	mushroom->motion.ax = 0;
	mushroom->motion.ay = 0;

	if (mushroom->position.x < 0 || mushroom->position.x > game->camera_pos + CAMERA_SIZE || mushroom->position.y > GROUND_LEVEL) {
		mushroom->state.active = 0;
	}
}

void process_goomba_logic(Entity *goomba, Game *game) {
	if (!goomba->state.active)
		return;

	goomba->motion.ay = GRAVITY;

	goomba->motion.vx = (goomba->render.flip == 0) ? -MAX_SPEED_H * 0.5 : MAX_SPEED_H * 0.5;

	for (int i = 0; i < MAX_ENTITIES; i++) {
		Entity *other = &game->entities[i];
		if (other == NULL || !other->state.active || other == goomba) continue;

		enum contact contactType = hitbox_contact(goomba, other);
		if (contactType != NONE) {
			switch (other->state.type) {
				case TYPE_MARIO_SMALL:
				case TYPE_MARIO_LARGE:
					if (contactType == UP) {

						goomba->state.state = STATE_DEAD;
						goomba->state.active = 0;
						other->motion.vy = -JUMP_INIT_V_SMALL;
					} else {

						if (other->state.state != STATE_ENLARGE) {
							other->state.state = (other->state.type == TYPE_MARIO_SMALL) ? STATE_DEAD : STATE_HIT;
						}
					}
					break;
				case TYPE_GROUND:
					if (contactType == DOWN) {
						goomba->motion.vy = 0;
						goomba->position.y = other->position.y - goomba->position.height;
					}
					break;
				case TYPE_TUBE:
				case TYPE_BLOCK_A:
				case TYPE_BLOCK_B_1:
				case TYPE_BLOCK_B_2:
				case TYPE_BLOCK_B_3:
				case TYPE_BLOCK_B_4:
				case TYPE_BLOCK_B_16:
				case TYPE_BLOCK_A_H_8:
				case TYPE_BLOCK_OBJ_C:
				case TYPE_BLOCK_OBJ_M:

					if (contactType == LEFT || contactType == RIGHT) {
						goomba->render.flip = (goomba->render.flip == 0) ? 1 : 0;
						goomba->motion.vx = -goomba->motion.vx;
					}
					break;
			}
		}
	}

	goomba->motion.vy += goomba->motion.ay;
	goomba->position.x += goomba->motion.vx;
	goomba->position.y += goomba->motion.vy;
	goomba->motion.ax = 0;
	goomba->motion.ay = 0;

	if (goomba->position.x < 0 || goomba->position.x > game->camera_pos + CAMERA_SIZE || goomba->position.y > GROUND_LEVEL) {
		goomba->state.active = 0;
	}
}

void process_block_logic(Entity *block, Game *game) {
	for (int i = 0; i < MAX_ENTITIES; i++) {
		Entity *entity = &game->entities[i];
		if (entity == block || !entity->state.active) continue;

		if (entity->state.type == TYPE_MARIO_SMALL || entity->state.type == TYPE_MARIO_LARGE) {
			enum contact type = hitbox_contact(entity, block);
			if (type == UP) {

				if ((block->state.type == TYPE_BLOCK_A || block->state.type == TYPE_BLOCK_OBJ_M) && entity->state.type == TYPE_MARIO_LARGE) {
					block->state.active = 0;
					continue;
				}

				if (block->state.type == TYPE_BLOCK_OBJ_C || block->state.type == TYPE_BLOCK_OBJ_M) {
					int entityAdded = 0;
					for (int j = 0; j < MAX_ENTITIES && !entityAdded; j++) {
						if (!game->entities[j].state.active) {
							Entity *newItem = &game->entities[j];
							newItem->position.x = block->position.x;
							newItem->position.y = block->position.y - newItem->position.height;
							newItem->state.active = 1;
							newItem->state.state = STATE_NORMAL;
							newItem->state.type = (block->state.type == TYPE_BLOCK_OBJ_C) ? TYPE_COIN : TYPE_MUSHROOM;
							entityAdded = 1;
						}
					}

					if (!entityAdded || block->state.type == TYPE_BLOCK_OBJ_M) {
						block->state.active = 0;
					}
				}

				if (block->state.type == TYPE_BLOCK_A) {
					block->state.state = BLOCK_ANIMATE;
				}
			}
		}
	}
}

int main() {
	pthread_t input_thread;
	Game game;
	const char *device_path = "/dev/vga_ball";
	int frame_select = 0;

	if ((vga_ball_fd = open(device_path, O_RDWR)) < 0) {
		fprintf(stderr, "Failed to open hardware device: %s\n", device_path);
		return EXIT_FAILURE;
	}

	if ((keyboard = openkeyboard(&endpoint_address)) == NULL) {
		fprintf(stderr, "Did not find a keyboard\n");
		exit(EXIT_FAILURE);
	}

	if (pthread_create(&input_thread, NULL, input_thread_function, NULL) != 0) {
		fprintf(stderr, "Failed to create input thread\n");
		return EXIT_FAILURE;
	}

	new_game(&game);

	while (1) {
		Entity *mario = &game.entities[0];
		if (mario->position.x > game.camera_pos + CAMERA_SIZE / 2 + LOAD_LIMIT) {
			game.camera_pos = mario->position.x - CAMERA_SIZE / 2 - LOAD_LIMIT;
			game.camera_pos = (game.camera_pos > 0) ? game.camera_pos : 0;
		}

		if (mario->state.state == STATE_DEAD) {
			//printf("Mario Died!\n");
			new_game(&game);
			continue;
		}

		entity_activation_update(&game, game.camera_pos);

		for (int i = 0; i < MAX_ENTITIES; i++) {
			Entity *entity = &game.entities[i];

			if (!entity) continue;

			if (entity->state.active) {
				switch (entity->state.type) {
					case TYPE_MARIO_SMALL:
					case TYPE_MARIO_LARGE:
						process_mario_logic(entity, &game);
						break;
					case TYPE_MUSHROOM:
						process_mushroom_logic(entity, &game);
						break;
					case TYPE_GOOMBA:
						process_goomba_logic(entity, &game);
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
						process_block_logic(entity, &game);
						break;
					case TYPE_COIN:
						break;
					case TYPE_TUBE:
						break;
					case TYPE_CLOUD:
						break;
					case TYPE_GROUND:
						break;
					default:
						break;
				}
			}

			animate_entity(&game, entity, frame_counter);
		}
		flush_frame(&game, frame_select);
		frame_select = !frame_select;

		usleep(1000);
	}

	pthread_cancel(input_thread);
	pthread_join(input_thread, NULL);
	close(vga_ball_fd);
	return 0;
}