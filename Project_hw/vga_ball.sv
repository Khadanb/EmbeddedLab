/*
 * Avalon memory-mapped peripheral that generates VGA
 *
 * Stephen A. Edwards
 * Columbia University
 */

module vga_ball(
    input logic        clk,
    input logic        reset,
    input logic [31:0] writedata,
    input logic        write,
    input logic        chipselect,
    input logic [2:0]  address,

    input logic        left_chan_ready,
    input logic        right_chan_ready,

    output logic [15:0] sample_data_l,
    output logic        sample_valid_l,
    output logic [15:0] sample_data_r,
    output logic        sample_valid_r,

    output logic [7:0]  VGA_R, VGA_G, VGA_B,
    output logic        VGA_CLK, VGA_HS, VGA_VS,
    output logic        VGA_BLANK_n,
    output logic        VGA_SYNC_n
);

    logic [10:0] hcount;
    logic [9:0]  vcount;
    logic [31:0] ppu_info, sound_buff;
    vga_counters counters(.clk50(clk), .*);

    always_ff @(posedge clk) begin
        if (reset) begin
            ppu_info <= 32'd_0;
            sound_buff <= 32'd_0;
        end else if (chipselect && write) begin
            case (address)
                3'h0: ppu_info <= writedata;
                3'h1: sound_buff <= writedata;
            endcase
        end
    end

    // Sound processing logic
    reg [13:0] counter;
    logic flag1, flag2, flag3, flag4;
    reg [9:0] address1, address2, address3, address4;
    wire [15:0] sound_output1, sound_output2, sound_output3, sound_output4;

    smb_breakblock_ROM breakblock_audio(.address(address1), .clock(clk), .q(sound_output1));
    smb_jump_ROM jump_audio(.address(address2), .clock(clk), .q(sound_output2));
    smb_gameover_ROM gameover_audio(.address(address3), .clock(clk), .q(sound_output3));
    smb_coin_ROM coin_audio(.address(address4), .clock(clk), .q(sound_output4));

    always_ff @(posedge clk) begin
        if (reset) begin
            counter <= 0;
            sample_valid_l <= 0;
            sample_valid_r <= 0;
            address1 <= 0;
            address2 <= 0;
            address3 <= 0;
            address4 <= 0;
            flag1 <= 0;
            flag2 <= 0;
            flag3 <= 0;
            flag4 <= 0;
        end else if (left_chan_ready && right_chan_ready) begin
            if (counter < 6250) begin
                counter <= counter + 1;
                sample_valid_l <= 0;
                sample_valid_r <= 0;
            end else begin
                counter <= 0;
                sample_valid_l <= 1;
                sample_valid_r <= 1;
                // Sound channel processing
                if (sound_buff[2:0] == 3'd3 && !flag3 || flag3 == 1'b0) begin
                    if (address3 < 10'd1000) begin
                        address3 <= address3 + 1;
                    end else begin
                        address3 <= 0;
                        flag3 <= 1'b1;
                    end
                    sample_data_l <= sound_output3;
                    sample_data_r <= sound_output3;
                end
                if (sound_buff[2:0] == 3'd1 && !flag1 || flag1 == 1'b0) begin
                    if (address1 < 10'd1000) begin
                        address1 <= address1 + 1;
                    end else begin
                        address1 <= 0;
                        flag1 <= 1'b1;
                    end
                    sample_data_l <= sound_output1;
                    sample_data_r <= sound_output1;
                end
                if (sound_buff[2:0] == 3'd2 && !flag2 || flag2 == 1'b0) begin
                    if (address2 < 10'd1000) begin
                        address2 <= address2 + 1;
                    end else begin
                        address2 <= 0;
                        flag2 <= 1'b1;
                    end
                    sample_data_l <= sound_output2;
                    sample_data_r <= sound_output2;
                end
                if (sound_buff[2:0] == 3'd4 && !flag4 || flag4 == 1'b0) begin
                    if (address4 < 10'd1000) begin
                        address4 <= address4 + 1;
                    end else begin
                        address4 <= 0;
                        flag4 <= 1'b1;
                    end
                    sample_data_l <= sound_output4;
                    sample_data_r <= sound_output4;
                end
            end
        end else begin
            sample_valid_l <= 0;
            sample_valid_r <= 0;
        end
    end

    // PPU for video output logic
    logic [23:0] PPU_out;
    ppu game_ppu(.clk(clk), .reset(reset), .writedata(ppu_info), .address(3'd_0), .hcount(hcount[10:1]),
                 .vcount(vcount), .RGB_output(PPU_out));

    // VGA signal generation
    always_comb begin
        {VGA_R, VGA_G, VGA_B} = (VGA_BLANK_n && hcount >= 160 && hcount < 1120 && vcount < 400) ? PPU_out : 24'h000000;
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
