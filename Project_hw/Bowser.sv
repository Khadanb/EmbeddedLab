module Bowser_display (
    input logic        clk,
    input logic        reset,
    input logic [31:0] writedata,
    input logic [9:0]  hcount,
    input logic [9:0]  vcount,
    output logic [23:0] RGB_output
);

   // Change for other type=========================
    parameter [5:0] sub_comp_ID = 6'b001001;
    parameter [4:0] pattern_num = 5'd_1;
    parameter [15:0] addr_limit = 16'd_4096;
	parameter [4:0] child_limit = 5'd_1;
	logic [3:0] mem [0:4095];
	logic [23:0] color_plate [0:4];
    logic [79:0] pattern_table [0:2]; 

    assign pattern_table[0] = {16'd_0, 16'd_32, 16'd_24, 16'd_32, 16'd_24};

    assign color_plate[0] = 24'hFFCC66;  // Light Brown
    assign color_plate[1] = 24'h33CC33;  // Green
    assign color_plate[2] = 24'hFFFFFF;  // White
    assign color_plate[3] = 24'h000000;  // Black
    assign color_plate[4] = 24'h202022;  // Dark Gray

    //=============================================
    
    // logic [79:0] ping_pong_pattern_input[0:1];
    logic [23:0] buffer_RGB_output[0:1];
    logic [15:0] buffer_addr_output[0:1];
    logic        buffer_addr_out_valid[0:1];
    logic [111:0] buffer_stateholder[0:1];
    logic        buffer_select = 1'b0;

	logic [5:0] sub_comp;
	logic [4:0] child_comp;
	logic [3:0] info;
	logic [2:0] input_type;
	logic [12:0] input_msg;
	logic		select;

	assign sub_comp = writedata[31:26];
	assign child_comp = writedata[25:21];
	assign info = writedata[20:17];
	assign input_type = writedata[16:14];
	assign select = writedata[13];
	assign input_msg = writedata[12:0];

	// loop ============================
	integer i, j, k;
	// ================================  

    //==========================================
    // Address_calculater change to adapt different setting
    //=============================================
	addr_cal AC_ping_0(.pattern_info(buffer_stateholder[0][111:32]), .sprite_info(buffer_stateholder[0][31:0]), .hcount(hcount), .vcount(vcount), .addr_output(buffer_addr_output[0]), .valid(buffer_addr_out_valid[0]));
    addr_cal AC_pong_0(.pattern_info(buffer_stateholder[1][111:32]), .sprite_info(buffer_stateholder[1][31:0]), .hcount(hcount), .vcount(vcount), .addr_output(buffer_addr_output[1]), .valid(buffer_addr_out_valid[1]));
    // FF Input================================
	always_ff @(posedge clk) begin
        case (info)
            // 4'b1111 to flush *PING(0)/PONG(1)* buffer and clear *THE OPPOSIT* buffer
            4'b1111: begin
                buffer_select = select;
                buffer_stateholder[~select][31] = 1'b0;
            end

            // 4;b0001 normal write to state holder **buffer_select(select)**
	        4'h0001 : begin
                // *****Change the sub_comp code to match the input
            	if (sub_comp == sub_comp_ID) begin
                    case (input_type)
                        3'b001: begin
                            // visible
                            buffer_stateholder[select][31] = input_msg[12];
                            // fliped
                            buffer_stateholder[select][30] = input_msg[11];
                            // pattern code
                            if (input_msg[4:0] < pattern_num)
                                buffer_stateholder[select][111:32] = pattern_table[input_msg[4:0]];
                        end
                        3'b010: begin
                            // x_coordinate
                            buffer_stateholder[select][29:20] = input_msg[9:0];
                        end
                        3'b011: begin
                            // y_coordinate
                            buffer_stateholder[select][19:10] = input_msg[9:0];
                        end
                        3'b100: begin
                            // shift_amount
                            buffer_stateholder[select][9:0] = input_msg[9:0];
                        end
                    endcase
		        end
            end
       endcase
	end

        // Output =====================================

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
		
		RGB_output = buffer_addr_out_valid[buffer_select]? buffer_RGB_output[buffer_select] : 24'h202020;
	end
		
initial begin
	$readmemh("/user/stud/fall22/hy2759/4840/pro_test/lab3-hw/on_chip_mem/Bowser.txt", mem);
end

   
endmodule