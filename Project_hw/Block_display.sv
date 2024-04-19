/*
 * Avalon memory-mapped peripheral that generates VGA
 *
 * Stephen A. Edwards
 * Columbia University
 */

module Block_display (input logic        clk,
	    input logic 	    reset,
		input logic [31:0]  writedata,
		input logic [9:0]   hcount,
		input logic [9:0]   vcount,

		output logic [23:0]	RGB_output);

    // Change for other type=========================
    parameter [5:0] sub_comp_ID = 6'b000010;
    parameter [4:0] pattern_num = 5'd_17;
    parameter [15:0] addr_limit = 16'd_2304;
	parameter [4:0] child_limit = 5'd_9;
	logic [3:0] mem [0:2303];
	logic [23:0] color_plate [0:9];
    logic [79:0] pattern_table [0:16]; 

    assign pattern_table[0] = {16'd_0, 16'd_16, 16'd_16, 16'd_16, 16'd_16};
    assign pattern_table[1] = {16'd_256, 16'd_16, 16'd_16, 16'd_16, 16'd_16};
    assign pattern_table[2] = {16'd_512, 16'd_16, 16'd_16, 16'd_16, 16'd_16};
    assign pattern_table[3] = {16'd_768, 16'd_16, 16'd_16, 16'd_16, 16'd_16};
    assign pattern_table[4] = {16'd_1024, 16'd_16, 16'd_16, 16'd_16, 16'd_16};
    assign pattern_table[5] = {16'd_1280, 16'd_16, 16'd_16, 16'd_16, 16'd_16};
    assign pattern_table[6] = {16'd_1536, 16'd_16, 16'd_16, 16'd_16, 16'd_16};
    assign pattern_table[7] = {16'd_1792, 16'd_16, 16'd_16, 16'd_16, 16'd_16};
    assign pattern_table[8] = {16'd_2048, 16'd_16, 16'd_16, 16'd_16, 16'd_16};
    assign pattern_table[9] = {16'd_1280, 16'd_16, 16'd_16, 16'd_32, 16'd_16};
    assign pattern_table[10] = {16'd_1280, 16'd_16, 16'd_16, 16'd_48, 16'd_16};
    assign pattern_table[11] = {16'd_1280, 16'd_16, 16'd_16, 16'd_128, 16'd_16};
    assign pattern_table[12] = {16'd_2048, 16'd_16, 16'd_16, 16'd_16, 16'd_32};
    assign pattern_table[13] = {16'd_2048, 16'd_16, 16'd_16, 16'd_16, 16'd_48};
    assign pattern_table[14] = {16'd_2048, 16'd_16, 16'd_16, 16'd_16, 16'd_64};
    assign pattern_table[15] = {16'd_2048, 16'd_16, 16'd_16, 16'd_64, 16'd_64};
	assign pattern_table[16] = {16'd_2048, 16'd_16, 16'd_16, 16'd_16, 16'd_64};

	assign color_plate[0] = 24'h9290ff;
	assign color_plate[1] = 24'h908fff;
	assign color_plate[2] = 24'h9a4b00;
	assign color_plate[3] = 24'he69a25;
	assign color_plate[4] = 24'h000000;
	assign color_plate[5] = 24'h974f00;
	assign color_plate[6] = 24'h572200;
	assign color_plate[7] = 24'h1a9300;
	assign color_plate[8] = 24'hfce1ce;
	assign color_plate[9] = 24'hffcdc4;

    //=============================================
    
    // logic [79:0] ping_pong_pattern_input[0:1];
    logic [23:0] ping_pong_RGB_output[0:1][0:8];
    logic [15:0] ping_pong_addr_output[0:1][0:8];
    logic        ping_pong_addr_out_valid[0:1][0:8];
    logic [111:0] ping_pong_stateholder[0:1][0:8];
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
    addr_cal AC_ping_2(.pattern_info(ping_pong_stateholder[0][2][111:32]), .sprite_info(ping_pong_stateholder[0][2][31:0]), .hcount(hcount), .vcount(vcount), .addr_output(ping_pong_addr_output[0][2]), .valid(ping_pong_addr_out_valid[0][2]));
    addr_cal AC_ping_3(.pattern_info(ping_pong_stateholder[0][3][111:32]), .sprite_info(ping_pong_stateholder[0][3][31:0]), .hcount(hcount), .vcount(vcount), .addr_output(ping_pong_addr_output[0][3]), .valid(ping_pong_addr_out_valid[0][3]));
    addr_cal AC_ping_4(.pattern_info(ping_pong_stateholder[0][4][111:32]), .sprite_info(ping_pong_stateholder[0][4][31:0]), .hcount(hcount), .vcount(vcount), .addr_output(ping_pong_addr_output[0][4]), .valid(ping_pong_addr_out_valid[0][4]));
    addr_cal AC_ping_5(.pattern_info(ping_pong_stateholder[0][5][111:32]), .sprite_info(ping_pong_stateholder[0][5][31:0]), .hcount(hcount), .vcount(vcount), .addr_output(ping_pong_addr_output[0][5]), .valid(ping_pong_addr_out_valid[0][5]));
    addr_cal AC_ping_6(.pattern_info(ping_pong_stateholder[0][6][111:32]), .sprite_info(ping_pong_stateholder[0][6][31:0]), .hcount(hcount), .vcount(vcount), .addr_output(ping_pong_addr_output[0][6]), .valid(ping_pong_addr_out_valid[0][6]));
    addr_cal AC_ping_7(.pattern_info(ping_pong_stateholder[0][7][111:32]), .sprite_info(ping_pong_stateholder[0][7][31:0]), .hcount(hcount), .vcount(vcount), .addr_output(ping_pong_addr_output[0][7]), .valid(ping_pong_addr_out_valid[0][7]));
    addr_cal AC_ping_8(.pattern_info(ping_pong_stateholder[0][8][111:32]), .sprite_info(ping_pong_stateholder[0][8][31:0]), .hcount(hcount), .vcount(vcount), .addr_output(ping_pong_addr_output[0][8]), .valid(ping_pong_addr_out_valid[0][8]));

    addr_cal AC_pong_0(.pattern_info(ping_pong_stateholder[1][0][111:32]), .sprite_info(ping_pong_stateholder[1][0][31:0]), .hcount(hcount), .vcount(vcount), .addr_output(ping_pong_addr_output[1][0]), .valid(ping_pong_addr_out_valid[1][0]));
    addr_cal AC_pong_1(.pattern_info(ping_pong_stateholder[1][1][111:32]), .sprite_info(ping_pong_stateholder[1][1][31:0]), .hcount(hcount), .vcount(vcount), .addr_output(ping_pong_addr_output[1][1]), .valid(ping_pong_addr_out_valid[1][1]));
    addr_cal AC_pong_2(.pattern_info(ping_pong_stateholder[1][2][111:32]), .sprite_info(ping_pong_stateholder[1][2][31:0]), .hcount(hcount), .vcount(vcount), .addr_output(ping_pong_addr_output[1][2]), .valid(ping_pong_addr_out_valid[1][2]));
    addr_cal AC_pong_3(.pattern_info(ping_pong_stateholder[1][3][111:32]), .sprite_info(ping_pong_stateholder[1][3][31:0]), .hcount(hcount), .vcount(vcount), .addr_output(ping_pong_addr_output[1][3]), .valid(ping_pong_addr_out_valid[1][3]));
    addr_cal AC_pong_4(.pattern_info(ping_pong_stateholder[1][4][111:32]), .sprite_info(ping_pong_stateholder[1][4][31:0]), .hcount(hcount), .vcount(vcount), .addr_output(ping_pong_addr_output[1][4]), .valid(ping_pong_addr_out_valid[1][4]));
    addr_cal AC_pong_5(.pattern_info(ping_pong_stateholder[1][5][111:32]), .sprite_info(ping_pong_stateholder[1][5][31:0]), .hcount(hcount), .vcount(vcount), .addr_output(ping_pong_addr_output[1][5]), .valid(ping_pong_addr_out_valid[1][5]));
    addr_cal AC_pong_6(.pattern_info(ping_pong_stateholder[1][6][111:32]), .sprite_info(ping_pong_stateholder[1][6][31:0]), .hcount(hcount), .vcount(vcount), .addr_output(ping_pong_addr_output[1][6]), .valid(ping_pong_addr_out_valid[1][6]));
    addr_cal AC_pong_7(.pattern_info(ping_pong_stateholder[1][7][111:32]), .sprite_info(ping_pong_stateholder[1][7][31:0]), .hcount(hcount), .vcount(vcount), .addr_output(ping_pong_addr_output[1][7]), .valid(ping_pong_addr_out_valid[1][7]));
    addr_cal AC_pong_8(.pattern_info(ping_pong_stateholder[1][8][111:32]), .sprite_info(ping_pong_stateholder[1][8][31:0]), .hcount(hcount), .vcount(vcount), .addr_output(ping_pong_addr_output[1][8]), .valid(ping_pong_addr_out_valid[1][8]));
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
			ping_pong_RGB_output[0][j] =  (ping_pong_addr_output[0][j] < addr_limit)? color_plate[mem[ping_pong_addr_output[0][j]]] : color_plate[mem[0]];

			ping_pong_RGB_output[1][j] =  (ping_pong_addr_output[1][j] < addr_limit)? color_plate[mem[ping_pong_addr_output[1][j]]] : color_plate[mem[0]];
		end
		
		RGB_output = 24'h9290ff;
		for (k = 0; k < child_limit; k = k + 1) begin
			if ((ping_pong_RGB_output[ping_pong][k] != 24'h9290ff) && ping_pong_addr_out_valid[ping_pong][k]) begin
				RGB_output = ping_pong_RGB_output[ping_pong][k];
				break;
			end
		end
	end
		
initial begin
	$readmemh("/user/stud/fall22/hy2759/4840/pro_test/lab3-hw/on_chip_mem/Block.txt", mem);
end

   
endmodule
