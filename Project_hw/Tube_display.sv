/*
 * Avalon memory-mapped peripheral that generates VGA
 *
 * Stephen A. Edwards
 * Columbia University
 */

module Tube_display (input logic        clk,
	    input logic 	    reset,
		input logic [31:0]  writedata,
		input logic [9:0]   hcount,
		input logic [9:0]   vcount,

		output logic [23:0]	RGB_output);

    
    parameter [5:0] COMPONENT_ID = 6'b001010; 
    parameter [4:0] pattern_num = 5'd_2;
    parameter [15:0] addr_limit = 16'd_576;  
	parameter [4:0] child_limit = 5'd_2;
	logic [3:0] mem [0:287];
	logic [23:0] color_plate [0:3];
    logic [79:0] pattern_table [0:1]; 

    assign pattern_table[0] = {16'd_0, 16'd_32, 16'd_16, 16'd_32, 16'd_16};
	assign pattern_table[1] = {16'd_544, 16'd_32, 16'd_1, 16'd_32, 16'd_128};

	assign color_plate[0] = 24'h202020;
	assign color_plate[1] = 24'h000000;
	assign color_plate[2] = 24'h8c0000;
	assign color_plate[3] = 24'h100000;

    
    
    
    logic [23:0] buffer_RGB_output[0:1][0:1];
    logic [15:0] buffer_addr_output[0:1][0:1];
    logic        buffer_addr_out_valid[0:1][0:1];
    logic [111:0] buffer_stateholder[0:1][0:1];
    logic        buffer = 1'b0;

	logic [5:0] sub_comp;
	logic [4:0] child_comp;
	logic [3:0] info;
	logic [2:0] input_type;
	logic [12:0] input_msg;
	logic		buffer_select;

	assign sub_comp = writedata[31:26];
	assign child_comp = writedata[25:21];
	assign info = writedata[20:17];
	assign input_type = writedata[16:14];
	assign buffer_select = writedata[13];
	assign input_msg = writedata[12:0];

	
	integer i, j, k;
	

    
    
    
	addr_cal AC_ping_0(.pattern_info(buffer_stateholder[0][0][111:32]), .sprite_info(buffer_stateholder[0][0][31:0]), .hcount(hcount), .vcount(vcount), .addr_output(buffer_addr_output[0][0]), .valid(buffer_addr_out_valid[0][0]));
    addr_cal AC_ping_1(.pattern_info(buffer_stateholder[0][1][111:32]), .sprite_info(buffer_stateholder[0][1][31:0]), .hcount(hcount), .vcount(vcount), .addr_output(buffer_addr_output[0][1]), .valid(buffer_addr_out_valid[0][1]));

    addr_cal AC_pong_0(.pattern_info(buffer_stateholder[1][0][111:32]), .sprite_info(buffer_stateholder[1][0][31:0]), .hcount(hcount), .vcount(vcount), .addr_output(buffer_addr_output[1][0]), .valid(buffer_addr_out_valid[1][0]));
    addr_cal AC_pong_1(.pattern_info(buffer_stateholder[1][1][111:32]), .sprite_info(buffer_stateholder[1][1][31:0]), .hcount(hcount), .vcount(vcount), .addr_output(buffer_addr_output[1][1]), .valid(buffer_addr_out_valid[1][1]));


    
	always_ff @(posedge clk) begin
        case (info)
            
            4'b1111: begin
                buffer = buffer_select;
				for (i = 0; i < child_limit; i = i + 1) begin
					buffer_stateholder[~buffer_select][i][31] = 1'b0;
				end 

            end

            
	        4'h0001 : begin
                
            	if (sub_comp == COMPONENT_ID) begin
					if (child_comp < child_limit) begin
		                case (input_type)
		                    3'b001: begin
		                        // visible
		                        buffer_stateholder[buffer_select][child_comp][31] = input_msg[12];
		                        // flipped
		                        buffer_stateholder[buffer_select][child_comp][30] = input_msg[11];
		                        // pattern code
		                        if (input_msg[4:0] < pattern_num)
		                            buffer_stateholder[buffer_select][child_comp][111:32] = pattern_table[input_msg[4:0]];
		                    end
		                    3'b010: begin
		                        // x_coordinate
		                        buffer_stateholder[buffer_select][child_comp][29:20] = input_msg[9:0];
		                    end
		                    3'b011: begin
		                        // y_coordinate
		                        buffer_stateholder[buffer_select][child_comp][19:10] = input_msg[9:0];
		                    end
		                    3'b100: begin
		                        // shift_amount
		                        buffer_stateholder[buffer_select][child_comp][9:0] = input_msg[9:0];
		                    end
		                endcase
					end
		        end
            end
       endcase
	end


        

	always_comb begin
		for (j = 0; j < child_limit; j = j + 1) begin
			buffer_RGB_output[0][j] =  (buffer_addr_output[0][j] < addr_limit)? 
			(
				(buffer_addr_output[0][j][0])? 
					color_plate[mem[(buffer_addr_output[0][j][15:1])][3:2]] :
					color_plate[mem[(buffer_addr_output[0][j][15:1])][1:0]]
			) :
			color_plate[mem[0]];
			
			buffer_RGB_output[1][j] =  (buffer_addr_output[1][j] < addr_limit)? 
			(
				(buffer_addr_output[1][j][0])? 
					color_plate[mem[(buffer_addr_output[1][j][15:1])][3:2]] :
					color_plate[mem[(buffer_addr_output[1][j][15:1])][1:0]]
			) :
			color_plate[mem[0]];
		end
		
		RGB_output = 24'h202020;
		for (k = 0; k < child_limit; k = k + 1) begin
			if ((buffer_RGB_output[buffer][k] != 24'h202020) && buffer_addr_out_valid[buffer][k]) begin
				RGB_output = buffer_RGB_output[buffer][k];
				break;
			end
		end
	end
		
initial begin
	$readmemh("/user/stud/fall22/hy2759/4840/pro_test/lab3-hw/on_chip_mem/Tube_2bit.txt", mem);
end

   
endmodule
