/*
 * Avalon memory-mapped peripheral that generates VGA
 *
 * Stephen A. Edwards
 * Columbia University
 */

module Mario_display (input logic        clk,
		input logic 	    reset,
		input logic [31:0]  writedata,
		input logic [9:0]   hcount,
		input logic [9:0]   vcount,

		output logic [23:0]	RGB_output);

	parameter [5:0] COMPONENT_ID = 6'b000001;
	parameter [4:0] pattern_num = 5'd_19;
	parameter [15:0] addr_limit = 16'd_7168;
	logic [3:0] mem [0:3583];
	logic [23:0] color_plate [0:3];
	logic [79:0] pattern_table [0:18];

	assign pattern_table[0] = {16'd_0, 16'd_16, 16'd_16, 16'd_16, 16'd_16};
	assign pattern_table[1] = {16'd_256, 16'd_16, 16'd_16, 16'd_16, 16'd_16};
	assign pattern_table[2] = {16'd_512, 16'd_16, 16'd_16, 16'd_16, 16'd_16};
	assign pattern_table[3] = {16'd_768, 16'd_16, 16'd_16, 16'd_16, 16'd_16};
	assign pattern_table[4] = {16'd_1024, 16'd_16, 16'd_16, 16'd_16, 16'd_16};
	assign pattern_table[5] = {16'd_1280, 16'd_16, 16'd_16, 16'd_16, 16'd_16};
	assign pattern_table[6] = {16'd_1536, 16'd_16, 16'd_16, 16'd_16, 16'd_16};
	assign pattern_table[7] = {16'd_1792, 16'd_16, 16'd_32, 16'd_16, 16'd_32};
	assign pattern_table[8] = {16'd_2304, 16'd_16, 16'd_32, 16'd_16, 16'd_32};
	assign pattern_table[9] = {16'd_2816, 16'd_16, 16'd_32, 16'd_16, 16'd_32};
	assign pattern_table[10] = {16'd_3328, 16'd_16, 16'd_32, 16'd_16, 16'd_32};
	assign pattern_table[11] = {16'd_3840, 16'd_16, 16'd_32, 16'd_16, 16'd_32};
	assign pattern_table[12] = {16'd_4352, 16'd_16, 16'd_32, 16'd_16, 16'd_32};
	assign pattern_table[13] = {16'd_4864, 16'd_16, 16'd_24, 16'd_16, 16'd_24};
	assign pattern_table[14] = {16'd_5248, 16'd_16, 16'd_24, 16'd_16, 16'd_24};
	assign pattern_table[15] = {16'd_5632, 16'd_16, 16'd_32, 16'd_16, 16'd_32};
	assign pattern_table[16] = {16'd_6144, 16'd_16, 16'd_16, 16'd_16, 16'd_16};
	assign pattern_table[17] = {16'd_6400, 16'd_16, 16'd_16, 16'd_16, 16'd_16};
	assign pattern_table[18] = {16'd_6656, 16'd_16, 16'd_32, 16'd_16, 16'd_32};

	assign color_plate[0] = 24'h202020;
	assign color_plate[1] = 24'hb53120;
	assign color_plate[2] = 24'h6b6d00;
	assign color_plate[3] = 24'hea9e22;

	logic [23:0] buffer_RGB_output[0:1];
	logic [15:0] buffer_addr_output[0:1];
	logic        buffer_addr_out_valid[0:1];
	logic [111:0] buffer_stateholder[0:1];
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

	addr_cal AC_ping_0(.pattern_info(buffer_stateholder[0][111:32]), .sprite_info(buffer_stateholder[0][31:0]), .hcount(hcount), .vcount(vcount), .addr_output(buffer_addr_output[0]), .valid(buffer_addr_out_valid[0]));
	addr_cal AC_pong_0(.pattern_info(buffer_stateholder[1][111:32]), .sprite_info(buffer_stateholder[1][31:0]), .hcount(hcount), .vcount(vcount), .addr_output(buffer_addr_output[1]), .valid(buffer_addr_out_valid[1]));

	always_ff @(posedge clk) begin
		case (info)

			4'b1111: begin
				buffer = buffer_select;
				buffer_stateholder[~buffer_select][31] = 1'b0;
			end

			4'h0001 : begin

				if (sub_comp == COMPONENT_ID) begin
					case (input_type)
						3'b001: begin
							// visible
							buffer_stateholder[buffer_select][31] = input_msg[12];
							// fliped
							buffer_stateholder[buffer_select][30] = input_msg[11];
							// pattern code
							if (input_msg[4:0] < pattern_num)
								buffer_stateholder[buffer_select][111:32] = pattern_table[input_msg[4:0]];
						end
						3'b010: begin
							// x_coordinate
							buffer_stateholder[buffer_select][29:20] = input_msg[9:0];
						end
						3'b011: begin
							// y_coordinate
							buffer_stateholder[buffer_select][19:10] = input_msg[9:0];
						end
						3'b100: begin
							// shift_amount
							buffer_stateholder[buffer_select][9:0] = input_msg[9:0];
						end
					endcase
				end
			end
	   endcase
	end

	always_comb begin
		buffer_RGB_output[0] =  (buffer_addr_output[0] < addr_limit)?
			(
				(buffer_addr_output[0][0])?
					color_plate[mem[(buffer_addr_output[0][15:1])][3:2]] :
					color_plate[mem[(buffer_addr_output[0][15:1])][1:0]]
			) :
			color_plate[mem[0]];

		buffer_RGB_output[1] =  (buffer_addr_output[1] < addr_limit)?
			(
				(buffer_addr_output[1][0])?
					color_plate[mem[(buffer_addr_output[1][15:1])][3:2]] :
					color_plate[mem[(buffer_addr_output[1][15:1])][1:0]]
			) :
			color_plate[mem[0]];

		RGB_output = buffer_addr_out_valid[buffer]? buffer_RGB_output[buffer] : 24'h202020;
	end

initial begin
	$readmemh("/user/stud/fall21/bk2746/Projects/EmbeddedLab/Project_hw/on_chip_mem/Mario_2bit.txt", mem);
end


endmodule
