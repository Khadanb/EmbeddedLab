/*
 * Avalon memory-mapped peripheral that generates VGA
 *
 * Stephen A. Edwards
 * Columbia University
 */

module Cloud_display (input logic        clk,
	    input logic 	    reset,
		input logic [31:0]  writedata,
		input logic [9:0]   hcount,
		input logic [9:0]   vcount,

		output logic [23:0]	RGB_output);

    // Change for other type=========================
    parameter [5:0] sub_comp_ID = 6'b001110; // #14
    parameter [4:0] pattern_num = 5'd_1;
    parameter [15:0] addr_limit = 16'd_768;  // 768
	parameter [4:0] child_limit = 5'd_1;
	logic [3:0] mem [0:383];
	logic [23:0] color_plate [0:3];
    logic [79:0] pattern_table [0:2]; 

    assign pattern_table[0] = {16'd_0, 16'd_32, 16'd_24, 16'd_32, 16'd_24};

	assign color_plate[0] = 24'h9290ff;
	assign color_plate[1] = 24'h000000;
	assign color_plate[2] = 24'hffffff;
	assign color_plate[3] = 24'h63adff;

    //=============================================
    
    // logic [79:0] ping_pong_pattern_input[0:1];
    logic [23:0] ping_pong_RGB_output[0:1];
    logic [15:0] ping_pong_addr_output[0:1];
    logic        ping_pong_addr_out_valid[0:1];
    logic [111:0] ping_pong_stateholder[0:1];
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
	addr_cal AC_ping_0(.pattern_info(ping_pong_stateholder[0][111:32]), .sprite_info(ping_pong_stateholder[0][31:0]), .hcount(hcount), .vcount(vcount), .addr_output(ping_pong_addr_output[0]), .valid(ping_pong_addr_out_valid[0]));
    addr_cal AC_pong_0(.pattern_info(ping_pong_stateholder[1][111:32]), .sprite_info(ping_pong_stateholder[1][31:0]), .hcount(hcount), .vcount(vcount), .addr_output(ping_pong_addr_output[1]), .valid(ping_pong_addr_out_valid[1]));
    // FF Input================================
	always_ff @(posedge clk) begin
        case (info)
            // 4'b1111 to flush *PING(0)/PONG(1)* buffer and clear *THE OPPOSIT* buffer
            4'b1111: begin
                ping_pong = pp_selc;
                ping_pong_stateholder[~pp_selc][31] = 1'b0;
            end

            // 4;b0001 normal write to state holder **ping_pong(pp_selc)**
	        4'h0001 : begin
                // *****Change the sub_comp code to match the input
            	if (sub_comp == sub_comp_ID) begin
                    case (input_type)
                        3'b001: begin
                            // visible
                            ping_pong_stateholder[pp_selc][31] = input_msg[12];
                            // fliped
                            ping_pong_stateholder[pp_selc][30] = input_msg[11];
                            // pattern code
                            if (input_msg[4:0] < pattern_num)
                                ping_pong_stateholder[pp_selc][111:32] = pattern_table[input_msg[4:0]];
                        end
                        3'b010: begin
                            // x_coordinate
                            ping_pong_stateholder[pp_selc][29:20] = input_msg[9:0];
                        end
                        3'b011: begin
                            // y_coordinate
                            ping_pong_stateholder[pp_selc][19:10] = input_msg[9:0];
                        end
                        3'b100: begin
                            // shift_amount
                            ping_pong_stateholder[pp_selc][9:0] = input_msg[9:0];
                        end
                    endcase
		        end
            end
       endcase
	end

        // Output =====================================

	always_comb begin
		ping_pong_RGB_output[0] =  (ping_pong_addr_output[0] < addr_limit)? 
			(
				(ping_pong_addr_output[0][0])? 
					color_plate[mem[(ping_pong_addr_output[0][15:1])][3:2]] :
					color_plate[mem[(ping_pong_addr_output[0][15:1])][1:0]]
			) :
			color_plate[mem[0]];
			
		ping_pong_RGB_output[1] =  (ping_pong_addr_output[1] < addr_limit)? 
			(
				(ping_pong_addr_output[1][0])? 
					color_plate[mem[(ping_pong_addr_output[1][15:1])][3:2]] :
					color_plate[mem[(ping_pong_addr_output[1][15:1])][1:0]]
			) :
			color_plate[mem[0]];
		
	// assign ping_pong_RGB_output[0] =  (ping_pong_addr_output[0] < addr_limit)? color_plate[mem[ping_pong_addr_output[0]]] : color_plate[mem[0]];
                                    
	// assign ping_pong_RGB_output[1] =  (ping_pong_addr_output[1] < addr_limit)? color_plate[mem[ping_pong_addr_output[1]]] : color_plate[mem[0]];
                                    
		RGB_output = ping_pong_addr_out_valid[ping_pong]? ping_pong_RGB_output[ping_pong] : 24'h9290ff;
	end
		
initial begin
	$readmemh("/user/stud/fall21/bk2746/Projects/EmbeddedLab/Project_hw/on_chip_mem/Cloud_2bit.txt", mem);
end

   
endmodule
