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

int vga_ball_fd;

int block_index;

int info_001 = 1;
int info_010 = 2;
int info_011 = 3;
int info_100 = 4;

int frame_counter = 0;

int can_jump = 0;

int bowser_alive = 1; 
int lives = 2; 
float speed = 0.5; 

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

	if (frame_counter % 100 == 0)
		printf("Flushing MARIO - Visible: %d, Flip: %d, X: %d, Y: %d, Pattern: %d\n", visible, flip, x, y, pattern_code);

	write_to_hardware(vga_ball_fd, 0, (int)((1 << 26) + (1 << 17) + (info_001 << 14) + (frame_select << 13) + (visible << 12) + (flip << 11) + (pattern_code & 0x1F)));
	write_to_hardware(vga_ball_fd, 0, (int)((1 << 26) + (1 << 17) + (info_010 << 14) + (frame_select << 13) + (x & 0x3FF)));
	write_to_hardware(vga_ball_fd, 0, (int)((1 << 26) + (1 << 17) + (info_011 << 14) + (frame_select << 13) + (y & 0x3FF)));
}

void flush_goomba(const Entity *entity, int frame_select) {

	int visible = entity->render.visible;
	int flip = entity->render.flip;
	int x = entity->position.x;
	int y = entity->position.y;
	int pattern_code = entity->render.pattern_code;
	if (frame_counter % 100 == 0)
		printf("Flushing GOOMBA - Visible: %d, Flip: %d, X: %d, Y: %d, Pattern: %d\n", visible, flip, x, y, pattern_code);

	write_to_hardware(vga_ball_fd, 0, (int)((5 << 26) + (1 << 17) + (info_001 << 14) + (frame_select << 13) + (visible << 12) + (flip << 11) + (pattern_code & 0x1F)));
	write_to_hardware(vga_ball_fd, 0, (int)((5 << 26) + (1 << 17) + (info_010 << 14) + (frame_select << 13) + (x & 0x3FF)));
	write_to_hardware(vga_ball_fd, 0, (int)((5 << 26) + (1 << 17) + (info_011 << 14) + (frame_select << 13) + (y & 0x3FF)));
}

void flush_block(const Entity *entity, int frame_select) {

	int visible = entity->render.visible;
	int flip = entity->render.flip;
	int x = entity->position.x;
	int y = entity->position.y;
	int pattern_code = entity->render.pattern_code;
	int entityTypeCode = entity->state.type;

	if (frame_counter % 100 == 0)
		printf("Flushing BLOCK - Type: %d, Visible: %d, Flip: %d, X: %d, Y: %d, Pattern: %d\n", entityTypeCode, visible, flip, x, y, pattern_code);

	write_to_hardware(vga_ball_fd, 0, (int)((7 << 26) + ((block_index & 0x1F) << 21) + (1 << 17) + (info_001 << 14) + (frame_select << 13) + (visible << 12) + (0 << 11) + (pattern_code & 0x1F)));
	write_to_hardware(vga_ball_fd, 0, (int)((7 << 26) + ((block_index & 0x1F) << 21) + (1 << 17) + (info_010 << 14) + (frame_select << 13) + (x & 0x3FF)));
	write_to_hardware(vga_ball_fd, 0, (int)((7 << 26) + ((block_index & 0x1F) << 21) + (1 << 17) + (info_011 << 14) + (frame_select << 13) + (y & 0x3FF)));
	block_index += 1;
}

void flush_peach(const Entity *entity, int frame_select) {
	int visible = entity->render.visible;
	int flip = entity->render.flip;
	int x = entity->position.x;
	int y = entity->position.y;
	int pattern_code = entity->render.pattern_code;

	if (frame_counter % 100 == 0)
		printf("Flushing Peach - Visible: %d, Flip: %d, X: %d, Y: %d, Pattern: %d\n", visible, flip, x, y, pattern_code);

	write_to_hardware(vga_ball_fd, 0, (int)((10 << 26) + (1 << 17) + (info_001 << 14) + (frame_select << 13) + (visible << 12) + (flip << 11) + (pattern_code & 0x1F)));
	write_to_hardware(vga_ball_fd, 0, (int)((10 << 26) + (1 << 17) + (info_010 << 14) + (frame_select << 13) + (x & 0x3FF)));
	write_to_hardware(vga_ball_fd, 0, (int)((10 << 26) + (1 << 17) + (info_011 << 14) + (frame_select << 13) + (y & 0x3FF)));
}

void flush_fireball(const Entity *entity, int frame_select) {
	int visible = entity->render.visible;
	int flip = entity->render.flip;
	int x = entity->position.x;
	int y = entity->position.y;
	int pattern_code = entity->render.pattern_code;

	if (frame_counter % 100 == 0)
		printf("Flushing Fireball - Visible: %d, Flip: %d, X: %d, Y: %d, Pattern: %d\n", visible, flip, x, y, pattern_code);

	write_to_hardware(vga_ball_fd, 0, (int)((8 << 26) + (1 << 17) + (info_001 << 14) + (frame_select << 13) + (visible << 12) + (flip << 11) + (pattern_code & 0x1F)));
	write_to_hardware(vga_ball_fd, 0, (int)((8 << 26) + (1 << 17) + (info_010 << 14) + (frame_select << 13) + (x & 0x3FF)));
	write_to_hardware(vga_ball_fd, 0, (int)((8 << 26) + (1 << 17) + (info_011 << 14) + (frame_select << 13) + (y & 0x3FF)));
}


void flush_tube(const Entity *entity, int frame_select) {
	int visible = entity->render.visible;
	int flip = entity->render.flip;
	int x = entity->position.x;
	int y = entity->position.y;
	int pattern_code = entity->render.pattern_code;

	if (frame_counter % 100 == 0)
		printf("Flushing TUBE - Visible: %d, Flip: %d, X: %d, Y: %d, Pattern: %d\n", visible, flip, x, y, pattern_code);

	write_to_hardware(vga_ball_fd, 0, (int)((15 << 26) + ((0&0x1F) << 21) + (1 << 17) + (info_001 << 14) + (frame_select << 13) + (1 << 12) + (flip << 11) + (ANI_TUBE_H & 0x1F)));
	write_to_hardware(vga_ball_fd, 0, (int)((15 << 26) + ((0&0x1F) << 21) + (1 << 17) + (info_010 << 14) + (frame_select << 13) + (x & 0x3FF)));
	write_to_hardware(vga_ball_fd, 0, (int)((15 << 26) + ((0&0x1F) << 21) + (1 << 17) + (info_011 << 14) + (frame_select << 13) + (y & 0x3FF)));

	write_to_hardware(vga_ball_fd, 0, (int)((15 << 26) + ((1&0x1F) << 21) + (1 << 17) + (info_001 << 14) + (frame_select << 13) + (1 << 12) + (flip << 11) + (ANI_TUBE_B & 0x1F)));
	write_to_hardware(vga_ball_fd, 0, (int)((15 << 26) + ((1&0x1F) << 21) + (1 << 17) + (info_010 << 14) + (frame_select << 13) + (x & 0x3FF)));
	write_to_hardware(vga_ball_fd, 0, (int)((15 << 26) + ((1&0x1F) << 21) + (1 << 17) + (info_011 << 14) + (frame_select << 13) + (y & 0x3FF)));
}

void flush_bowser(const Entity *entity, int frame_select) {
	int visible = entity->render.visible;
	int flip = entity->render.flip;
	int x = entity->position.x;
	int y = entity->position.y;
	int pattern_code = entity->render.pattern_code;

	if (frame_counter % 100 == 0)
		printf("Flushing Bowser - Visible: %d, Flip: %d, X: %d, Y: %d, Pattern: %d\n", visible, flip, x, y, pattern_code);

	write_to_hardware(vga_ball_fd, 0, (int)((9 << 26) + (1 << 17) + (info_001 << 14) + (frame_select << 13) + (visible << 12) + (flip << 11) + (pattern_code & 0x1F)));
	write_to_hardware(vga_ball_fd, 0, (int)((9 << 26) + (1 << 17) + (info_010 << 14) + (frame_select << 13) + (x & 0x3FF)));
	write_to_hardware(vga_ball_fd, 0, (int)((9 << 26) + (1 << 17) + (info_011 << 14) + (frame_select << 13) + (y & 0x3FF)));
}

void flush_ground(Entity *entity, int camera_pos, int frame_select) {
	int visible = entity->render.visible;
	int flip = entity->render.flip;
	int x = entity->position.x;
	int y = entity->position.y;
	int pattern_code = entity->render.pattern_code;
	int left_edge = entity->position.x + entity->position.width;
	int right_edge = left_edge + GROUND_PIT_WIDTH;

	if (frame_counter % 100 == 0)
		printf("Flushing Ground - Visible: %d, Flip: %d, X: %d, Y: %d, Pattern: %d, l: %d. r:%d\n", visible, flip, x, y, pattern_code, left_edge, right_edge);

	write_to_hardware(vga_ball_fd, 0, (int)((14 << 26) + ((0&0x1F) << 21) + (1 << 17) + (info_001 << 14) + (frame_select << 13) + (visible << 12) + (0 << 11) + (0 & 0x1F)));
	write_to_hardware(vga_ball_fd, 0, (int)((14 << 26) + ((0&0x1F) << 21) + (1 << 17) + (info_010 << 14) + (frame_select << 13) + ((15 - (camera_pos%16)) & 0x3FF)));
	write_to_hardware(vga_ball_fd, 0, (int)((14 << 26) + ((0&0x1F) << 21) + (1 << 17) + (info_011 << 14) + (frame_select << 13) + (0 & 0x3FF)));
	write_to_hardware(vga_ball_fd, 0, (int)((14 << 26) + ((0&0x1F) << 21) + (1 << 17) + (info_100 << 14) + (frame_select << 13) + (0 & 0x3FF)));
}

void flush_entity(Entity *entity, int frame_select, int camera_pos) {
	if (entity->render.pattern_code > 6 || entity->render.pattern_code < 0 ) return;
	if (entity->state.type > TYPE_EMP || entity->state.type < 0) return;
	if (entity->state.active != 1) return;

	switch (entity->state.type) {
		case TYPE_MARIO_SMALL:
		case TYPE_MARIO_LARGE:
			flush_mario(entity, frame_select);
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
		case TYPE_TUBE:
			flush_tube(entity, frame_select);
			break;
		case TYPE_BOWSER:
			flush_bowser(entity,frame_select);
			break;
		case TYPE_GROUND:
			flush_ground(entity, camera_pos, frame_select);

			break;
		case TYPE_PEACH:
			flush_peach(entity, frame_select);
			break;
		case TYPE_FIREBALL:
			flush_fireball(entity, frame_select);
			break; 
		default:
			break;
	}
}

void flush_frame(Game *game, int frame_select) {
	int entity_index;
	block_index = 0;
	Entity *entity;

	for (entity_index = 0; entity_index < MAX_ENTITIES; entity_index++) {
		entity = &game->entities[entity_index];

		if (entity->state.active == 1 && entity->render.visible == 1) {
			flush_entity(entity, frame_select, game->camera_pos);
		}
	}

	write_to_hardware(vga_ball_fd, 0, (int)((1 << 26) + (0xf << 17) + (frame_select << 13)));

	frame_counter = (frame_counter >= FRAME_LIMIT) ? 0 : frame_counter + 1;
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

void handle_collision_with_block(Entity *mario, Entity *other, enum contact type) {
	if (type == UP) {
		mario->motion.vy = 0;
	} else if (type == DOWN) {
		mario->motion.vy = 0;
		mario->position.y = other->position.y - mario->position.height;
		can_jump = 1;
	} else if (type == LEFT && mario->render.flip == 1) {
		mario->motion.vx = 0;
	} else if (type == RIGHT && mario->render.flip == 0) {
		mario->motion.vx = 0;
	}
}

void handle_collision_with_tube(Entity *mario, Entity *other, enum contact type) {
	if (type == LEFT && mario->render.flip == 1) {
		mario->motion.vx = 0;
	} else if(type == RIGHT && mario->render.flip == 0) {
		mario->motion.vx = 0;
	} else if (type == DOWN) {
		mario->motion.vy = 0;
		mario->position.y = other->position.y - mario->position.height;
		can_jump = 1;
	}
}

void handle_collision_with_ground(Entity *mario, Entity *other, enum contact type) {
	if (type == DOWN) {
		mario->motion.vy = 0;
		mario->position.y = other->position.y - mario->position.height;
		can_jump = 1;
	}
}

void process_mario_logic(Entity *mario, Game *game) {
	if (mario == NULL) {
		printf("Mario entity is NULL\n");
		return;
	}

	// Apply gravity
	mario->motion.ay = GRAVITY;

	// Reset horizontal acceleration
	mario->motion.ax = 0;

	// Handle horizontal input
	if (current_key == KEY_LEFT) {
		mario->motion.ax = -WALK_ACC;
		mario->render.flip = 1; // Mario faces left
	} else if (current_key == KEY_RIGHT) {
		mario->motion.ax = WALK_ACC;
		mario->render.flip = 0; // Mario faces right
	}

	// Handle jump input
	if (current_key == KEY_JUMP && can_jump) {
		mario->motion.vy = -JUMP_INIT_V_LARGE;
		can_jump = 0;
	}

	// Apply friction if Mario is on the ground and moving
	if (mario->motion.vy == 0 && fabs(mario->motion.vx) > MOTION_MIN) {
		mario->motion.ax -= mario->motion.vx * FRICTION;
	}

	if (fabs(mario->motion.vx) < 0.08f) {
		mario->motion.vx = 0;
	}

	// Update velocities
	mario->motion.vx += mario->motion.ax;
	mario->motion.vy += mario->motion.ay;

	// Limit speeds
	mario->motion.vx = fminf(fmaxf(mario->motion.vx, -MAX_SPEED_H), MAX_SPEED_H);
	mario->motion.vy = fminf(fmaxf(mario->motion.vy, -MAX_SPEED_V_JUMP), MAX_SPEED_V);

	// Collision detection
	for (int i = 1; i < MAX_ENTITIES; i++) {
		Entity *other = &game->entities[i];
		if (other == NULL || !other->state.active) continue;
		enum contact contactType = hitbox_contact(mario, other);
		if (contactType != NONE) {
			switch (other->state.type) {
				case TYPE_GOOMBA:
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
				case TYPE_PEACH:
					mario->state.state = STATE_DEAD; 
				default:
					break;
			}
		}
	}

	mario->position.x += mario->motion.vx;
	game->camera_velocity = 0;
	if (mario->position.x > ((2*CAMERA_SIZE)/3)) {
		mario->position.x = ((2*CAMERA_SIZE)/3) - 1;
		game->camera_velocity = mario->motion.vx;

		if (game->camera_pos < 355 || bowser_alive == 0)
			game->camera_pos += mario->motion.vx;
		if (game->camera_pos < game->camera_start) {
			game->camera_pos = game->camera_start;
		}
	} else if (mario->position.x < 70) {
		mario->position.x = 70 + 1;
	}

	mario->position.y += mario->motion.vy;
	if (mario->position.y > GROUND_LEVEL) {
		mario->state.state = STATE_DEAD;
		mario->state.active = 0;
	}
}

void process_goomba_logic(Entity *goomba, Game *game) {
	enum contact contactType;
	Entity *other;
	if (!goomba->state.active)
		return;

	goomba->motion.ay = GRAVITY;

	goomba->motion.vx = (goomba->render.flip == 0) ? -MAX_SPEED_H * 0.5 : MAX_SPEED_H * 0.5;
	for (int i = 0; i < MAX_ENTITIES; i++) {
		other = &game->entities[i];
		if (other == NULL || !other->state.active || other == goomba) continue;

		contactType = hitbox_contact(goomba, other);
		if (contactType != NONE) {
			switch (other->state.type) {
				case TYPE_MARIO_SMALL:
					if (contactType == UP) {

						goomba->state.state = STATE_DEAD;
						goomba->state.active = 0;
						other->motion.vy = -JUMP_INIT_V_SMALL;
					} else {

						other->state.state = STATE_DEAD;
					}
					break;
				case TYPE_MARIO_LARGE:
					if (contactType == UP) {
						goomba->state.state = STATE_DEAD;
						goomba->state.active = 0;
						other->motion.vy = -JUMP_INIT_V_SMALL;
					} else {
						other->state.type = TYPE_MARIO_SMALL;
						other->state.state = STATE_HIT;
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

	if (goomba->position.y > GROUND_LEVEL) {
		goomba->state.state = STATE_DEAD;
		goomba->state.active = 0;
	}

	if (game->camera_pos < 355 || bowser_alive == 0) {
		goomba->position.x -= game->camera_velocity;
	}

	if (goomba->position.x < game->camera_start) {
		goomba->state.active = 0;
		printf("Cull Goomba");
	}

	// if(goomba->position.x > game->camera_start + CAMERA_SIZE) {
	// 	goomba->render.visible = 0; 
	// } else {
	// 	goomba->render.visible = 1;
	// }
}

void process_bowser_logic(Entity *bowser, Game *game) {
	enum contact contactType;
	Entity *other;
	if (!bowser->state.active)
		return;

	bowser->motion.ay = GRAVITY;

	bowser->motion.vx = (bowser->render.flip == 0) ? -MAX_SPEED_H * speed : MAX_SPEED_H * speed;
	for (int i = 0; i < MAX_ENTITIES; i++) {
		other = &game->entities[i];
		if (other == NULL || !other->state.active || other == bowser) continue;

		contactType = hitbox_contact(bowser, other);
		if (contactType != NONE) {
			switch (other->state.type) {
				case TYPE_MARIO_SMALL:
					if (contactType == UP) {

						if (lives == 0) {
							bowser->state.state = STATE_DEAD;
							bowser->state.active = 0;
							bowser->render.visible = 0;
							bowser_alive = 0;
						} else {
							lives -= 1;
							speed += 0.15;
						}

						other->motion.vy = -JUMP_INIT_V_LARGE;
					} else {

						other->state.state = STATE_DEAD;
					}
					break;
				case TYPE_MARIO_LARGE:
					if (contactType == UP) {
						bowser->state.state = STATE_DEAD;
						bowser->state.active = 0;
						other->motion.vy = -JUMP_INIT_V_SMALL;
					} else {
						other->state.type = TYPE_MARIO_SMALL;
						other->state.state = STATE_HIT;
					}
					break;
				case TYPE_GROUND:
					if (contactType == DOWN) {
						bowser->motion.vy = 0;
						bowser->position.y = other->position.y - bowser->position.height;
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
						bowser->render.flip = (bowser->render.flip == 0) ? 1 : 0;
						bowser->motion.vx = -bowser->motion.vx;
					}
					break;
			}
		}
	}

	bowser->motion.vy += bowser->motion.ay;
	bowser->position.x += bowser->motion.vx;
	bowser->position.y += bowser->motion.vy;
	bowser->motion.ax = 0;
	bowser->motion.ay = 0;

	if (bowser->position.y > GROUND_LEVEL) {
		// bowser->state.state = STATE_DEAD;
		// bowser->state.active = 0;
		// bowser->render.visible = 0;
		bowser->position.y = GROUND_LEVEL - 32;
	}

	if (game->camera_pos < 355 || bowser_alive == 0) {
		bowser->position.x -= game->camera_velocity;
	}	
		
	if (bowser->position.x < game->camera_start) {
		bowser->state.active = 0;
		printf("Cull Goomba");
	}

	// if(bowser->position.x > game->camera_start + CAMERA_SIZE) {
	// 	bowser->render.visible = 0; 
	// } else {
	// 	bowser->render.visible = 1;
	// }
}

void process_fireball_logic(Entity *fireball, Game *game) {
	enum contact contactType;
	Entity *other;
	if (!fireball->state.active)
		return;

	fireball->motion.ay = 0;

	fireball->motion.vx = (fireball->render.flip == 0) ? -MAX_SPEED_H : MAX_SPEED_H;
	for (int i = 0; i < MAX_ENTITIES; i++) {
		other = &game->entities[i];
		if (other == NULL || !other->state.active || other == fireball) continue;

		contactType = hitbox_contact(fireball, other);
		if (contactType != NONE) {
			switch (other->state.type) {
				case TYPE_MARIO_SMALL:
				case TYPE_MARIO_LARGE:
					other->state.state = STATE_DEAD;
					break;
			}
		}
	}

	fireball->motion.vy += fireball->motion.ay;
	fireball->position.x += fireball->motion.vx;
	fireball->position.y += fireball->motion.vy;
	fireball->motion.ax = 0;
	fireball->motion.ay = 0;

	if (fireball->position.y > GROUND_LEVEL) {
		fireball->state.state = STATE_DEAD;
		fireball->state.active = 0;
		fireball->render.visible = 0;
	}

	fireball->position.x -= game->camera_velocity;
	if (fireball->position.x < game->camera_start) {
		fireball->render.flip = !fireball->render.flip;
		fireball->position.x = game->camera_start + 15;
	} else if(fireball->position.x > game->camera_start + CAMERA_SIZE) {
		fireball->render.flip = !fireball->render.flip;
		fireball->position.x = game->camera_start + CAMERA_SIZE - 30; 
	}

	if(fireball->position.x > game->camera_start + CAMERA_SIZE) {
		fireball->render.visible = 0; 
	} else {
		fireball->render.visible = 1;
	}
}

int main() {
	pthread_t input_thread;
	Game game;
	const char *device_path = "/dev/vga_ball";
	int frame_select = 0;
	Entity *mario;
	Entity *entity;

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

		mario = &game.entities[0];
		if (mario->state.state == STATE_DEAD || current_key == KEY_NEWGAME) {
			new_game(&game);
			lives = 2;
			bowser_alive = 1; 
			speed = 0.5;
			continue;
		}

		for (int i = 0; i < MAX_ENTITIES; i++) {
			entity = &game.entities[i];

			if (!entity) continue;

			if (entity->state.active == 1) {
				switch (entity->state.type) {
					case TYPE_MARIO_SMALL:
					case TYPE_MARIO_LARGE:
						process_mario_logic(entity, &game);
						break;
					case TYPE_GOOMBA:
						process_goomba_logic(entity, &game);
						break;
					case TYPE_GROUND:
						// entity->position.x -= game.camera_velocity;

						// if (entity->position.x > game.camera_start + CAMERA_SIZE) {
						// 	entity->render.visible = 0;
						// } else {
						// 	entity->render.visible = 1;
						// }
						break;
					case TYPE_BOWSER:
						process_bowser_logic(entity, &game);
						break;
					case TYPE_FIREBALL:
						process_fireball_logic(entity, &game);
						break;
					default:
						if (game.camera_pos < 355 || bowser_alive == 0) {
							entity->position.x -= game.camera_velocity;
						}

						if(entity->position.x < game.camera_start) {
							entity->state.active = 0;
						}

						if(entity->position.x > game.camera_start + CAMERA_SIZE) {
							entity->render.visible = 0; 
						} else {
							entity->render.visible = 1;
						}
						break;
				}
			}

			animate_entity(&game, entity, frame_counter);
		}
		flush_frame(&game, frame_select);
		frame_select = !frame_select;

		if (frame_counter % 100 == 0)
			printf("camera pos = %d\n", game.camera_pos);
		usleep(16667);
	}

	pthread_cancel(input_thread);
	pthread_join(input_thread, NULL);
	close(vga_ball_fd);
	return 0;
}