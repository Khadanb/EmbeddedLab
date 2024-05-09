/*
 * Avalon memory-mapped peripheral that generates VGA
 *
 * Stephen A. Edwards
 * Columbia University
 */

module Ground_display (input logic        clk,
		input logic 	    reset,
		input logic [31:0]  writedata,
		input logic [9:0]   hcount,
		input logic [9:0]   vcount,

		output logic [23:0]	RGB_output);


	parameter [5:0] COMPONENT_ID = 6'b001111;
	parameter [4:0] pattern_num = 5'd_1;
	parameter [15:0] addr_limit = 16'd_256;
	logic [3:0] mem [0:127];
	logic [23:0] color_plate [0:3];
	logic [79:0] pattern_table [0:1];

	assign pattern_table[0] = {16'd_0, 16'd_16, 16'd_16, 16'd_650, 16'd_32};

	assign color_plate[0] = 24'h202020;
	assign color_plate[1] = 24'hFFFFFF;
	assign color_plate[2] = 24'h808080;
	assign color_plate[3] = 24'hD3D3D3;

	parameter [9:0] ground_height = 10'd_368;


	logic [23:0] buffer_RGB_out[0:1];
	logic [15:0] buffer_addr_out[0:1];
	logic        buffer_addr_valid[0:1];
	logic [111:0] frame_buffer_state[0:1];
	logic        buffer_select = 1'b0;

	logic [5:0] sub_comp;
	logic [4:0] child_comp;
	logic [3:0] info;
	logic [2:0] input_type;
	logic [12:0] input_msg;
	logic		buffer_state;

	assign sub_comp = writedata[31:26];
	assign child_comp = writedata[25:21];
	assign info = writedata[20:17];
	assign input_type = writedata[16:14];
	assign buffer_state = writedata[13];
	assign input_msg = writedata[12:0];

	assign frame_buffer_state[0][19:10] = ground_height;
	assign frame_buffer_state[1][19:10] = ground_height;
	logic [9:0] l_edge = 10'd0;
	logic [9:0] r_edge = 10'd0;

	addr_cal AC_ping_0(.pattern_info(frame_buffer_state[0][111:32]), .sprite_info(frame_buffer_state[0][31:0]), .hcount(hcount), .vcount(vcount), .addr_output(buffer_addr_out[0]), .valid(buffer_addr_valid[0]));
	addr_cal AC_pong_0(.pattern_info(frame_buffer_state[1][111:32]), .sprite_info(frame_buffer_state[1][31:0]), .hcount(hcount), .vcount(vcount), .addr_output(buffer_addr_out[1]), .valid(buffer_addr_valid[1]));

	always_ff @(posedge clk) begin
		case (info)

			4'b1111: begin
				buffer_select = buffer_state;
				frame_buffer_state[~buffer_state][31] = 1'b0;
			end

			4'h0001 : begin

				if (sub_comp == COMPONENT_ID) begin
					case (input_type)
						3'b001: begin
							// visible
							frame_buffer_state[buffer_state][31] = input_msg[12];
							// fliped
							frame_buffer_state[buffer_state][30] = input_msg[11];
							// pattern code
							if (input_msg[4:0] < pattern_num)
								frame_buffer_state[buffer_state][111:32] = pattern_table[input_msg[4:0]];
						end
						3'b010: begin
							// x_coordinate
							frame_buffer_state[buffer_state][29:20] = input_msg[9:0];
						end
						3'b011: begin

							l_edge = input_msg[9:0];
						end
						3'b100: begin

							r_edge = input_msg[9:0];
						end
					endcase
				end
			end
	   endcase
	end

	assign buffer_RGB_out[0] =  (buffer_addr_out[0] < addr_limit)?
			(
				(buffer_addr_out[0][0])?
					color_plate[mem[(buffer_addr_out[0][15:1])][3:2]] :
					color_plate[mem[(buffer_addr_out[0][15:1])][1:0]]
			) :
			color_plate[mem[0]];

	assign buffer_RGB_out[1] =  (buffer_addr_out[1] < addr_limit)?
			(
				(buffer_addr_out[1][0])?
					color_plate[mem[(buffer_addr_out[1][15:1])][3:2]] :
					color_plate[mem[(buffer_addr_out[1][15:1])][1:0]]
			) :
			color_plate[mem[0]];

	assign RGB_output = buffer_addr_valid[buffer_select]?
						(((hcount < l_edge)||(hcount > r_edge))? buffer_RGB_out[buffer_select] : 24'h202020)
						: 24'h202020;

initial begin
	$readmemh("/user/stud/fall21/bk2746/Projects/EmbeddedLab/Project_hw/on_chip_mem/Ground_2bit.txt", mem);
end


endmodule
