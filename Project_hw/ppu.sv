/*
 * Avalon memory-mapped peripheral that generates VGA
 *
 * Stephen A. Edwards
 * Columbia University
 */

module ppu (input logic        clk,
	        input logic 	   reset,
		input logic [31:0]  writedata,
		input logic [2:0]  address,
		input logic [9:0]  hcount,
		input logic [9:0]   vcount,

		output logic [23:0]	RGB_output);

	logic [23:0] background = 24'd_0;
	logic [23:0] RGB_list[0:40];

	// loop ============================
	integer i;
	// ================================  
	
	Mario_display Mario_0(.clk(clk), .reset(reset), .writedata(writedata), .hcount(hcount), .vcount(vcount), .RGB_output(RGB_list[3]));
	Block_display Block_0(.clk(clk), .reset(reset), .writedata(writedata), .hcount(hcount), .vcount(vcount), .RGB_output(RGB_list[7]));
	// Coin_display Coin_0(.clk(clk), .reset(reset), .writedata(writedata), .hcount(hcount), .vcount(vcount), .RGB_output(RGB_list[9]));
	// Cloud_display Cloud_0(.clk(clk), .reset(reset), .writedata(writedata), .hcount(hcount), .vcount(vcount), .RGB_output(RGB_list[16]));
	Tube_display Tube_0(.clk(clk), .reset(reset), .writedata(writedata), .hcount(hcount), .vcount(vcount), .RGB_output(RGB_list[15]));	
	// Mush_display Mush_0(.clk(clk), .reset(reset), .writedata(writedata), .hcount(hcount), .vcount(vcount), .RGB_output(RGB_list[12]));
	Ground_display Ground_0(.clk(clk), .reset(reset), .writedata(writedata), .hcount(hcount), .vcount(vcount), .RGB_output(RGB_list[14]));
	Goomba_display Goomba_0(.clk(clk), .reset(reset), .writedata(writedata), .hcount(hcount), .vcount(vcount), .RGB_output(RGB_list[5]));
	always_comb begin

		RGB_list[0] = 24'h202020;
		RGB_list[1] = 24'h202020;
		RGB_list[2] = 24'h202020;
		// RGB_list[3] = 24'h202020;	//Mario
		RGB_list[4] = 24'h202020;
		// RGB_list[5] = 24'h202020;	//Goomba
		RGB_list[6] = 24'h202020;
		// RGB_list[7] = 24'h202020;	//Block
		RGB_list[8] = 24'h202020;	
		RGB_list[9] = 24'h202020;	//Coin
		RGB_list[10] = 24'h202020;
		RGB_list[11] = 24'h202020;
		RGB_list[12] = 24'h202020;	//Mush
		RGB_list[13] = 24'h202020;
		// RGB_list[14] = 24'h202020;	//Ground
		// RGB_list[15] = 24'h202020;	//Tube
		// RGB_list[16] = 24'h202020;	//Cloud
		RGB_list[17] = 24'h202020;
		RGB_list[18] = 24'h202020;
		RGB_list[19] = 24'h202020;
		RGB_output = 24'h202020;
		RGB_output = 24'h202020;
		for (i = 0; i < 20; i = i + 1) begin
			if (RGB_list[i] != 24'h202020) begin
				RGB_output = RGB_list[i];
				break;		
			end
		end		
	end
   
endmodule
