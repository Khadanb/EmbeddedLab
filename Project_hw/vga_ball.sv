/*
 * Avalon memory-mapped peripheral that generates VGA
 *
 * Stephen A. Edwards
 * Columbia University
 */

module vga_ball(input logic        clk,
	        input logic 	   reset,
		input logic [31:0]  writedata,
		input logic 	   write,
		input 		   chipselect,
		input logic [2:0]  address,

		input left_chan_ready,
		input right_chan_ready,

		output logic [15:0] sample_data_l,
		output logic sample_valid_l,
		output logic [15:0] sample_data_r,
		output logic sample_valid_r,

		output logic [7:0] VGA_R, VGA_G, VGA_B,
		output logic 	   VGA_CLK, VGA_HS, VGA_VS,
		                   VGA_BLANK_n,
		output logic 	   VGA_SYNC_n);

   logic [10:0]	   hcount;
   logic [9:0]     vcount;
   logic [31:0] ppu_info,sound_buff;
	
   vga_counters counters(.clk50(clk), .*);

   always_ff @(posedge clk)
     if (reset) begin
	ppu_info <= 32'd_0;
	sound_buff <= 32'd_0;
     end else if (chipselect && write)
       case (address)
	 3'h0 : ppu_info <= writedata;
	 3'h1 : sound_buff <= writedata;// sound

       endcase

//----------------Sound-----------------
	reg [13:0] counter;
	logic flag1;
	logic flag2;
	logic flag3;
	logic flag4;

	reg	[9:0]  address1;
	wire [15:0]  q1;
	smb_breakblock_ROM audio1(.address(address1), .clock(clk), .q(q1));//12877

	reg	[9:0]  address2;
	wire [15:0]  q2;
	smb_jump_ROM  audio2(.address(address2), .clock(clk), .q(q2));//12066

	reg	[9:0]  address3;
	wire [15:0]  q3;
	//smb_gameover_ROM audio3(.address(address3), .clock(clk), .q(q3));//59806
	smb_gameover_ROM audio3(.address(address3), .clock(clk), .q(q3));//59806
	reg [9:0] address4;
	wire [15:0] q4;
	smb_coin_ROM audio4(.address(address4), .clock(clk), .q(q4));//20848
	
	always_ff @(posedge clk) begin
		if(reset) begin
			counter <= 0;
			sample_valid_l <= 0; sample_valid_r <= 0;
		end
		else if(left_chan_ready == 1 && right_chan_ready == 1 && counter < 6250) begin
			counter <= counter + 1;
			sample_valid_l <= 0; sample_valid_r <= 0;
		end
		else if(left_chan_ready == 1 && right_chan_ready == 1 && counter == 6250) begin
			counter <= 0;
			sample_valid_l <= 1; sample_valid_r <= 1;
			//------Game over------
			if (sound_buff[2:0]==3'd3 || flag3 ==1'b0) begin
				if (address3 < 10'd1000) begin   // 59806
					address3 <= address3+1;
					flag3 <= 1'b0;
				end
				else begin
					address3 <=0;
					flag3 <= 1'b1;
				end
				sample_data_l <= q3;
				sample_data_r <= q3;
			end
			//------Break Block------
			else if (sound_buff[2:0]==3'd1 || flag1 ==1'b0) begin
				if (address1 < 10'd1000) begin //12066
					address1 <= address1+1;
					flag1 <= 1'b0;
				end
				else begin
					address1 <=0;
					flag1 <= 1'b1;
				end
				sample_data_l <= q1;
				sample_data_r <= q1;
			end
			//------Jump Surper------
			else if (sound_buff[2:0]==3'd2 || flag2 ==1'b0) begin
				if (address2 < 10'd1000) begin //12877
					address2 <= address2+1;
					flag2 <= 1'b0;
				end
				else begin
					address2 <=0;
					flag2 <= 1'b1;
				end
				sample_data_l <= q2;
				sample_data_r <= q2;
			end
			//------coin------
			else if (sound_buff[2:0]==3'd4 || flag4 == 1'b0) begin
				if (address4 < 10'd1000) begin   //20848
					address4 <= address4 + 1;
					flag4 <= 1'b0;
				end else begin
					address4 <= 0;
					flag4 <= 1'b1;
				end
				sample_data_l <= q4;
				sample_data_r <= q4;
			end
			else if (sound_buff[2:0]==3'd5 ) begin
					address1 <= 0;
					address2 <= 0;
					address3 <= 0;
					address4 <= 0;
					flag1 <= 1'b0;
					flag2 <= 1'b0;
					flag3 <= 1'b0;
					flag4 <= 1'b0;
				
				sample_data_l <= 0;
				sample_data_r <= 0;
			end


			else begin
				sample_data_l <= 0;
				sample_data_r <= 0;
			end
		end
		else begin
		sample_valid_l <= 0; sample_valid_r <= 0;
		end
	end


   //test =========
   logic [23:0] PPU_out;
   ppu game_ppu(.clk(clk), .reset(reset), .writedata(ppu_info), .address(3'd_0), .hcount(hcount[10:1]),
 		.vcount(vcount), .RGB_output(PPU_out));
   //==============
	
	always_comb begin
		// Set default color to black
		{VGA_R, VGA_G, VGA_B} = {8'h00, 8'h00, 8'h00};

		// Update the RGB values only within the active video region and when VGA_BLANK_n is asserted
		if (VGA_BLANK_n && (hcount < 11'd_1120 && hcount >= 11'd_160 && vcount < 10'd_400)) begin
			{VGA_R, VGA_G, VGA_B} = PPU_out;
		end
	end

	       
endmodule

module vga_counters(
 input logic 	     clk50, reset,
 output logic [10:0] hcount,  // hcount[10:1] is pixel column
 output logic [9:0]  vcount,  // vcount[9:0] is pixel row
 output logic 	     VGA_CLK, VGA_HS, VGA_VS, VGA_BLANK_n, VGA_SYNC_n);

/*
 * 640 X 480 VGA timing for a 50 MHz clock: one pixel every other cycle
 * 
 * HCOUNT 1599 0             1279       1599 0
 *             _______________              ________
 * ___________|    Video      |____________|  Video
 * 
 * 
 * |SYNC| BP |<-- HACTIVE -->|FP|SYNC| BP |<-- HACTIVE
 *       _______________________      _____________
 * |____|       VGA_HS          |____|
 */
   // Parameters for hcount
   parameter HACTIVE      = 11'd 1280,
             HFRONT_PORCH = 11'd 32,
             HSYNC        = 11'd 192,
             HBACK_PORCH  = 11'd 96,   
             HTOTAL       = HACTIVE + HFRONT_PORCH + HSYNC +
                            HBACK_PORCH; // 1600
   
   // Parameters for vcount
   parameter VACTIVE      = 10'd 480,
             VFRONT_PORCH = 10'd 10,
             VSYNC        = 10'd 2,
             VBACK_PORCH  = 10'd 33,
             VTOTAL       = VACTIVE + VFRONT_PORCH + VSYNC +
                            VBACK_PORCH; // 525

   logic endOfLine;
   
   always_ff @(posedge clk50 or posedge reset)
     if (reset)          hcount <= 0;
     else if (endOfLine) hcount <= 0;
     else  	         hcount <= hcount + 11'd 1;

   assign endOfLine = hcount == HTOTAL - 1;
       
   logic endOfField;
   
   always_ff @(posedge clk50 or posedge reset)
     if (reset)          vcount <= 0;
     else if (endOfLine)
       if (endOfField)   vcount <= 0;
       else              vcount <= vcount + 10'd 1;

   assign endOfField = vcount == VTOTAL - 1;

   // Horizontal sync: from 0x520 to 0x5DF (0x57F)
   // 101 0010 0000 to 101 1101 1111
   assign VGA_HS = !( (hcount[10:8] == 3'b101) &
		      !(hcount[7:5] == 3'b111));
   assign VGA_VS = !( vcount[9:1] == (VACTIVE + VFRONT_PORCH) / 2);

   assign VGA_SYNC_n = 1'b0; // For putting sync on the green signal; unused
   
   // Horizontal active: 0 to 1279     Vertical active: 0 to 479
   // 101 0000 0000  1280	       01 1110 0000  480
   // 110 0011 1111  1599	       10 0000 1100  524
   assign VGA_BLANK_n = !( hcount[10] & (hcount[9] | hcount[8]) ) &
			!( vcount[9] | (vcount[8:5] == 4'b1111) );

   /* VGA_CLK is 25 MHz
    *             __    __    __
    * clk50    __|  |__|  |__|
    *        
    *             _____       __
    * hcount[0]__|     |_____|
    */
   assign VGA_CLK = hcount[0]; // 25 MHz clock: rising edge sensitive
   
endmodule
