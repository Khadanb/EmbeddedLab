/*
 * Userspace program that communicates with the vga_ball device driver
 * through ioctls
 *
 * Stephen A. Edwards
 * Columbia University
 */

#include <stdio.h>
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

int vga_ball_fd;

/* Read and print the background color */
void print_background_color() {
  vga_ball_arg_t vla;
  
  if (ioctl(vga_ball_fd, VGA_BALL_READ_BACKGROUND, &vla)) {
      perror("ioctl(VGA_BALL_READ_BACKGROUND) failed");
      return;
  }
  printf("%02x %02x %02x\n",
	 vla.background.red, vla.background.green, vla.background.blue);
}

/* Set the background color */
void set_background_color(const vga_ball_color_t *c, unsigned char xl, unsigned char xh, unsigned char yl, unsigned char yh)
{
  vga_ball_arg_t vla;
  vla.background = *c;
	vla.coordinate.xl = xl;
	vla.coordinate.xh = xh;
	vla.coordinate.yl = yl;
	vla.coordinate.yh = yh;
  if (ioctl(vga_ball_fd, VGA_BALL_WRITE_BACKGROUND, &vla)) {
      perror("ioctl(VGA_BALL_SET_BACKGROUND) failed");
      return;
  }
}

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

int main()
{
  vga_ball_arg_t vla;
  int i;
	uint16_t clk_count = 0;
  static const char filename[] = "/dev/vga_ball";
	unsigned char xl, xh, yl, yh;
	uint16_t x_16, y_16;
	uint16_t x_v, y_v;
	int8_t x_sign, y_sign;
	int bus_info;
	int ping_pong = 0;
	int info_001 = 1;
	int info_010 = 2;
	int info_011 = 3;
	int info_100 = 4;
	int flip = 0;
	int flip_count = 400;
	int pp = 0;
	int pp_count = 20;
	int coin_pp_count = 8;
	int block_pp;
	int coin_pp = 0;
	int goomba_flip = 0;
	int goomba_flip_count = 6;
	int ground_shift = 0;
	int ground_count = 7;

	//=========================
	int margin_width = 5 * 16;

  static const vga_ball_color_t colors[] = {
    { 0xff, 0x00, 0x00 }, /* Red */
    { 0x00, 0xff, 0x00 }, /* Green */
    { 0x00, 0x00, 0xff }, /* Blue */
    { 0xff, 0xff, 0x00 }, /* Yellow */
    { 0x00, 0xff, 0xff }, /* Cyan */
    { 0xff, 0x00, 0xff }, /* Magenta */
    { 0x80, 0x80, 0x80 }, /* Gray */
    { 0x00, 0x00, 0x00 }, /* Black */
    { 0xff, 0xff, 0xff }  /* White */
  };

# define COLORS 9

	time_t t;
	srand((unsigned) time(&t));

  printf("VGA ball Userspace program started\n");

  if ( (vga_ball_fd = open(filename, O_RDWR)) == -1) {
    fprintf(stderr, "could not open %s\n", filename);
    return -1;
  }
	x_16 = (uint16_t)(rand()%200 + 100);
	y_16 = (uint16_t)(rand()%100 + 100);
	xh = (unsigned char)((x_16>>8) & 0xFF);
	xl = (unsigned char)(x_16 & 0xFF);
	yh = (unsigned char)((y_16>>8) & 0xFF);
	yl = (unsigned char)(y_16 & 0xFF);
	x_v = 5 + rand()%5;
	y_v = 5 + rand()%5;

	vga_ball_color_t back = { 0x00, 0x00, 0xff };
	clk_count = 0;
	x_sign = 1;
	y_sign = 1;
	while (1){

		if (x_16 >= 450-45) x_sign = -1;
		else if(x_16 <= 45) x_sign = 1;

		if (y_16 >= 450-(45/2)) y_sign = -1;
		else if(y_16 <= 45/2) y_sign = 1;

		if(clk_count%x_v == 0){
			x_16 = x_sign == 1? x_16 + 1 : x_16 - 1;		
		}
		if(clk_count%y_v == 0){
			y_16 = y_sign == 1? y_16 + 1 : y_16 - 1;	
		}
		xh = (unsigned char)((x_16>>8) & 0xFF);
		xl = (unsigned char)(x_16 & 0xFF);
		yh = (unsigned char)((y_16>>8) & 0xFF);
		yl = (unsigned char)(y_16 & 0xFF);
		// bus_info = (x_16 & 0x3FF) << 10;
		// bus_info += (y_16 & 0x3FF);
		// write_2_hw(0, bus_info);

		if(clk_count%flip_count == 0){
			flip = flip? 0:1;	
		}
		//Mario =========================
		bus_info = (1 << 26);
		write_2_hw(0, (int)(bus_info + (1 << 17) + (info_001 << 14) + (ping_pong << 13) + (1 << 12) + (flip << 11) + (pp & 0x1F)));
		write_2_hw(0, (int)(bus_info + (1 << 17) + (info_010 << 14) + (ping_pong << 13) + ((margin_width + x_16) & 0x3FF)));
		write_2_hw(0, (int)(bus_info + (1 << 17) + (info_011 << 14) + (ping_pong << 13) + (y_16 & 0x3FF)));
		// write_2_hw(0, (int)(bus_info + (1 << 17) + (info_100 << 14) + (ping_pong << 13) + (0 & 0x3FF)));
		//Block =========================
		for(int i = 0; i < 9; i++){
			block_pp = i + 8;
			write_2_hw(0, (int)((2 << 26) + ((i&0x1F) << 21) + (1 << 17) + (info_001 << 14) + (ping_pong << 13) + (1 << 12) + (flip << 11) + (block_pp & 0x1F)));
			write_2_hw(0, (int)((2 << 26) + ((i&0x1F) << 21) + (1 << 17) + (info_010 << 14) + (ping_pong << 13) + (((i%4) * 64 + margin_width) & 0x3FF)));
			write_2_hw(0, (int)((2 << 26) + ((i&0x1F) << 21) + (1 << 17) + (info_011 << 14) + (ping_pong << 13) + (((int)(i/4)*64) & 0x3FF)));
			// write_2_hw(0, (int)((2 << 26) + ((i&0x1F) << 21) + (1 << 17) + (info_100 << 14) + (ping_pong << 13) + (8 & 0x3FF)));
		}	
		//Coin =========================
		for(int i = 0; i < 4; i++){
			write_2_hw(0, (int)((3 << 26) + ((i&0x1F) << 21) + (1 << 17) + (info_001 << 14) + (ping_pong << 13) + (1 << 12) + (flip << 11) + (coin_pp & 0x1F)));
			write_2_hw(0, (int)((3 << 26) + ((i&0x1F) << 21) + (1 << 17) + (info_010 << 14) + (ping_pong << 13) + ((margin_width + 350 + (i%4) * 32) & 0x3FF)));
			write_2_hw(0, (int)((3 << 26) + ((i&0x1F) << 21) + (1 << 17) + (info_011 << 14) + (ping_pong << 13) + (((int)(i/2)*64) & 0x3FF)));
			// write_2_hw(0, (int)((3 << 26) + ((i&0x1F) << 21) + (1 << 17) + (info_100 << 14) + (ping_pong << 13) + (0 & 0x3FF)));
		}

		//Cloud =========================
		for(int i = 0; i < 1; i++){
			write_2_hw(0, (int)((14 << 26) + ((i&0x1F) << 21) + (1 << 17) + (info_001 << 14) + (ping_pong << 13) + (1 << 12) + (flip << 11) + (0 & 0x1F)));
			write_2_hw(0, (int)((14 << 26) + ((i&0x1F) << 21) + (1 << 17) + (info_010 << 14) + (ping_pong << 13) + ((margin_width + 270 + (i%4) * 64) & 0x3FF)));
			write_2_hw(0, (int)((14 << 26) + ((i&0x1F) << 21) + (1 << 17) + (info_011 << 14) + (ping_pong << 13) + (((int)(i/4)*64 + 250) & 0x3FF)));
			// write_2_hw(0, (int)((3 << 26) + ((i&0x1F) << 21) + (1 << 17) + (info_100 << 14) + (ping_pong << 13) + (0 & 0x3FF)));
		}

		//Mush =========================
		for(int i = 0; i < 2; i++){
			write_2_hw(0, (int)((9 << 26) + ((i&0x1F) << 21) + (1 << 17) + (info_001 << 14) + (ping_pong << 13) + (1 << 12) + (flip << 11) + (i & 0x1F)));
			write_2_hw(0, (int)((9 << 26) + ((i&0x1F) << 21) + (1 << 17) + (info_010 << 14) + (ping_pong << 13) + ((margin_width + 270 + (i%4) * 32) & 0x3FF)));
			write_2_hw(0, (int)((9 << 26) + ((i&0x1F) << 21) + (1 << 17) + (info_011 << 14) + (ping_pong << 13) + (((int)(i/4)*64 + 180) & 0x3FF)));
			// write_2_hw(0, (int)((3 << 26) + ((i&0x1F) << 21) + (1 << 17) + (info_100 << 14) + (ping_pong << 13) + (0 & 0x3FF)));
		}

		//Tube =========================
		for(int i = 0; i < 1; i++){
			write_2_hw(0, (int)((10 << 26) + (((i*2)&0x1F) << 21) + (1 << 17) + (info_001 << 14) + (ping_pong << 13) + (1 << 12) + (flip << 11) + (0 & 0x1F)));
			write_2_hw(0, (int)((10 << 26) + (((i*2)&0x1F) << 21) + (1 << 17) + (info_010 << 14) + (ping_pong << 13) + ((margin_width + 20 + (i%4) * 64) & 0x3FF)));
			write_2_hw(0, (int)((10 << 26) + (((i*2)&0x1F) << 21) + (1 << 17) + (info_011 << 14) + (ping_pong << 13) + (((int)(i/4)*64 + 250) & 0x3FF)));
		////////////////////////////////////
			write_2_hw(0, (int)((10 << 26) + (((i*2 + 1)&0x1F) << 21) + (1 << 17) + (info_001 << 14) + (ping_pong << 13) + (1 << 12) + (flip << 11) + (1 & 0x1F)));
			write_2_hw(0, (int)((10 << 26) + (((i*2 + 1)&0x1F) << 21) + (1 << 17) + (info_010 << 14) + (ping_pong << 13) + ((margin_width + 20 + (i%4) * 64) & 0x3FF)));
			write_2_hw(0, (int)((10 << 26) + (((i*2 + 1)&0x1F) << 21) + (1 << 17) + (info_011 << 14) + (ping_pong << 13) + (((int)(i/4)*64 + 250) & 0x3FF)));
			// write_2_hw(0, (int)((3 << 26) + ((i&0x1F) << 21) + (1 << 17) + (info_100 << 14) + (ping_pong << 13) + (0 & 0x3FF)));
		}

		//Goomba =========================
		for(int i = 0; i < 2; i++){
			write_2_hw(0, (int)((5 << 26) + ((i&0x1F) << 21) + (1 << 17) + (info_001 << 14) + (ping_pong << 13) + (1 << 12) + (goomba_flip << 11) + (flip & 0x1F)));
			write_2_hw(0, (int)((5 << 26) + ((i&0x1F) << 21) + (1 << 17) + (info_010 << 14) + (ping_pong << 13) + ((margin_width + 350 + (i%4) * 32) & 0x3FF)));
			write_2_hw(0, (int)((5 << 26) + ((i&0x1F) << 21) + (1 << 17) + (info_011 << 14) + (ping_pong << 13) + (((int)(i/4)*64 + 180) & 0x3FF)));
			// write_2_hw(0, (int)((3 << 26) + ((i&0x1F) << 21) + (1 << 17) + (info_100 << 14) + (ping_pong << 13) + (0 & 0x3FF)));
		}

		//Ground ==========================
			write_2_hw(0, (int)((15 << 26) + ((0&0x1F) << 21) + (1 << 17) + (info_001 << 14) + (ping_pong << 13) + (1 << 12) + (0 << 11) + (0 & 0x1F)));
			write_2_hw(0, (int)((15 << 26) + ((0&0x1F) << 21) + (1 << 17) + (info_010 << 14) + (ping_pong << 13) + ((15 - (ground_shift%16)) & 0x3FF)));
			write_2_hw(0, (int)((15 << 26) + ((0&0x1F) << 21) + (1 << 17) + (info_011 << 14) + (ping_pong << 13) + (400 & 0x3FF)));
			write_2_hw(0, (int)((15 << 26) + ((0&0x1F) << 21) + (1 << 17) + (info_100 << 14) + (ping_pong << 13) + (450 & 0x3FF)));
		//Flush =========================	
		write_2_hw(0, (int)(bus_info + (0xf << 17) + (ping_pong << 13)));
		ping_pong = ping_pong? 0 : 1;
		
		if(clk_count%pp_count == 0){
			pp = (pp >= 18)? 0 : (pp+1);	
		}
		if(clk_count%coin_pp_count == 0){
			coin_pp = (coin_pp >= 3)? 0 : (coin_pp+1);	
		}
		if(clk_count%goomba_flip_count == 0){
			goomba_flip = goomba_flip? 0 : 1;	
		}
		if(clk_count%ground_count == 0){
			ground_shift  = (ground_shift >= 479)? 0 : ground_shift + 1;	
		}
		clk_count++;
		usleep(25000);
	}
  
  printf("VGA BALL Userspace program terminating\n");
  return 0;
}
