/*
 * Avalon memory-mapped peripheral that generates VGA
 *
 * Stephen A. Edwards
 * Columbia University
 */

module Goomba_display (input logic        clk,
	    input logic 	    reset,
		input logic [31:0]  writedata,
		input logic [9:0]   hcount,
		input logic [9:0]   vcount,

		output logic [23:0]	RGB_output);

    // Change for other type=========================
    parameter [5:0] sub_comp_ID = 6'b000101; // #5
    parameter [4:0] pattern_num = 5'd_2;
    parameter [15:0] addr_limit = 16'd_384;  // 384
	parameter [4:0] child_limit = 5'd_2;
	logic [3:0] mem [0:191];
	logic [23:0] color_plate [0:3];
    logic [79:0] pattern_table [0:1]; 

    assign pattern_table[0] = {16'd_0, 16'd_16, 16'd_16, 16'd_16, 16'd_16};
    assign pattern_table[1] = {16'd_256, 16'd_16, 16'd_8, 16'd_16, 16'd_8};

	assign color_plate[0] = 24'h000000;
	assign color_plate[1] = 24'h9c4a00;
	assign color_plate[2] = 24'h000000;
	assign color_plate[3] = 24'hffcec5;

    //=============================================
    
    // logic [79:0] ping_pong_pattern_input[0:1];
    logic [23:0] ping_pong_RGB_output[0:1][0:1];
    logic [15:0] ping_pong_addr_output[0:1][0:1];
    logic        ping_pong_addr_out_valid[0:1][0:1];
    logic [111:0] ping_pong_stateholder[0:1][0:1];
    logic        ping_pong = 1'b0;

	logic [5:0] sub_comp;
	logic [4:0] child_comp;
	logic [3:0] info;
	logic [2:0] input_type;
	logic [12:0] input_msg;
	logic		pp_selc;

	assign sub_comp = writedata[31:26];
	assign child_comp = writedata[25:21];
	assign info = writedata[20:17];
	assign input_type = writedata[16:14];
	assign pp_selc = writedata[13];
	assign input_msg = writedata[12:0];

	// loop ============================
	integer i, j, k;
	// ================================  

    //==========================================
    // Address_calculater change to adapt different setting
    //=============================================
	addr_cal AC_ping_0(.pattern_info(ping_pong_stateholder[0][0][111:32]), .sprite_info(ping_pong_stateholder[0][0][31:0]), .hcount(hcount), .vcount(vcount), .addr_output(ping_pong_addr_output[0][0]), .valid(ping_pong_addr_out_valid[0][0]));
    addr_cal AC_ping_1(.pattern_info(ping_pong_stateholder[0][1][111:32]), .sprite_info(ping_pong_stateholder[0][1][31:0]), .hcount(hcount), .vcount(vcount), .addr_output(ping_pong_addr_output[0][1]), .valid(ping_pong_addr_out_valid[0][1]));

    addr_cal AC_pong_0(.pattern_info(ping_pong_stateholder[1][0][111:32]), .sprite_info(ping_pong_stateholder[1][0][31:0]), .hcount(hcount), .vcount(vcount), .addr_output(ping_pong_addr_output[1][0]), .valid(ping_pong_addr_out_valid[1][0]));
    addr_cal AC_pong_1(.pattern_info(ping_pong_stateholder[1][1][111:32]), .sprite_info(ping_pong_stateholder[1][1][31:0]), .hcount(hcount), .vcount(vcount), .addr_output(ping_pong_addr_output[1][1]), .valid(ping_pong_addr_out_valid[1][1]));
    // FF Input================================
	always_ff @(posedge clk) begin
        case (info)
            // 4'b1111 to flush *PING(0)/PONG(1)* buffer and clear *THE OPPOSIT* buffer
            4'b1111: begin
                ping_pong = pp_selc;
				for (i = 0; i < child_limit; i = i + 1) begin
					ping_pong_stateholder[~pp_selc][i][31] = 1'b0;
				end 

            end

            // 4;b0001 normal write to state holder **ping_pong(pp_selc)**
	        4'h0001 : begin
                // *****Change the sub_comp code to match the input
            	if (sub_comp == sub_comp_ID) begin
					if (child_comp < child_limit) begin
		                case (input_type)
		                    3'b001: begin
		                        // visible
		                        ping_pong_stateholder[pp_selc][child_comp][31] = input_msg[12];
		                        // fliped
		                        ping_pong_stateholder[pp_selc][child_comp][30] = input_msg[11];
		                        // pattern code
		                        if (input_msg[4:0] < pattern_num)
		                            ping_pong_stateholder[pp_selc][child_comp][111:32] = pattern_table[input_msg[4:0]];
		                    end
		                    3'b010: begin
		                        // x_coordinate
		                        ping_pong_stateholder[pp_selc][child_comp][29:20] = input_msg[9:0];
		                    end
		                    3'b011: begin
		                        // y_coordinate
		                        ping_pong_stateholder[pp_selc][child_comp][19:10] = input_msg[9:0];
		                    end
		                    3'b100: begin
		                        // shift_amount
		                        ping_pong_stateholder[pp_selc][child_comp][9:0] = input_msg[9:0];
		                    end
		                endcase
					end
		        end
            end
       endcase
	end


        // Output =====================================

	always_comb begin
		for (j = 0; j < child_limit; j = j + 1) begin
			ping_pong_RGB_output[0][j] =  (ping_pong_addr_output[0][j] < addr_limit)? 
			(
				(ping_pong_addr_output[0][j][0])? 
					color_plate[mem[(ping_pong_addr_output[0][j][15:1])][3:2]] :
					color_plate[mem[(ping_pong_addr_output[0][j][15:1])][1:0]]
			) :
			color_plate[mem[0]];
			
			ping_pong_RGB_output[1][j] =  (ping_pong_addr_output[1][j] < addr_limit)? 
			(
				(ping_pong_addr_output[1][j][0])? 
					color_plate[mem[(ping_pong_addr_output[1][j][15:1])][3:2]] :
					color_plate[mem[(ping_pong_addr_output[1][j][15:1])][1:0]]
			) :
			color_plate[mem[0]];
		end
		
		RGB_output = 24'h000000;
		for (k = 0; k < child_limit; k = k + 1) begin
			if ((ping_pong_RGB_output[ping_pong][k] != 24'h000000) && ping_pong_addr_out_valid[ping_pong][k]) begin
				RGB_output = ping_pong_RGB_output[ping_pong][k];
				break;
			end
		end
	end
		
initial begin
	$readmemh("/user/stud/fall21/bk2746/Projects/EmbeddedLab/Project_hw/on_chip_mem/Goomba_2bit.txt", mem);
end

   
endmodule
