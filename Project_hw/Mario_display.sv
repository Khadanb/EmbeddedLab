/*
 * Avalon memory-mapped peripheral that generates VGA
 * 
 * Stephen A. Edwards
 * Columbia University
 */

module Mario_display (
    input logic        clk,
    input logic        reset,
    input logic [31:0] writedata,
    input logic [9:0]  hcount,
    input logic [9:0]  vcount,
    output logic [23:0] RGB_output
);

    // Configuration parameters
    parameter [5:0] COMPONENT_ID = 6'b000001;
    parameter [4:0] MAX_PATTERN_COUNT = 5'd_19;
    parameter [15:0] ADDRESS_LIMIT = 16'd_7168;
    logic [3:0] pixel_data [0:3583];
    logic [23:0] color_plate [0:3];
    logic [79:0] pattern_table [0:18]; 

    // Initialize pattern data
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

    // Initialize color palette
    assign color_plate[0] = 24'h202020; // Background color
	assign color_plate[1] = 24'hb53120; // Red variant
	assign color_plate[2] = 24'h6b6d00; // Green variant
	assign color_plate[3] = 24'hea9e22; // Yellow variant

    // Buffer control and output logic
    logic [23:0] buffer_color_output[0:1];
    logic [15:0] buffer_address_output[0:1];
    logic        buffer_valid[0:1];
    logic [111:0] buffer_state[0:1];
    logic        buffer_select = 1'b0;

    // Decode writedata fields
    logic [5:0] component;
    logic [4:0] subcomponent;
    logic [3:0] command;
    logic [2:0] command_type;
    logic [12:0] command_data;
    logic        buffer_toggle;

    // Decode input writedata
    assign component = writedata[31:26];
    assign subcomponent = writedata[25:21];
    assign command = writedata[20:17];
    assign command_type = writedata[16:14];
    assign buffer_toggle = writedata[13];
    assign command_data = writedata[12:0];

    // Address calculators for sprite rendering
    addr_cal address_calculator_0(
        .pattern_info(buffer_state[0][111:32]),
        .sprite_info(buffer_state[0][31:0]),
        .hcount(hcount),
        .vcount(vcount),
        .addr_output(buffer_address_output[0]),
        .valid(buffer_valid[0])
    );

    addr_cal address_calculator_1(
        .pattern_info(buffer_state[1][111:32]),
        .sprite_info(buffer_state[1][31:0]),
        .hcount(hcount),
        .vcount(vcount),
        .addr_output(buffer_address_output[1]),
        .valid(buffer_valid[1])
    );

    // Handle configuration and buffer management
    always_ff @(posedge clk) begin
        case (command)
            4'b1111: begin
                buffer_select = buffer_toggle;
                buffer_state[~buffer_toggle][31] = 1'b0;  // Clear the other buffer
            end
            4'h0001: begin
                if (component == COMPONENT_ID) begin
                    case (command_type)
                        3'b001: begin
                            buffer_state[buffer_toggle][31] = command_data[12];  // Visibility
                            buffer_state[buffer_toggle][30] = command_data[11];  // Flipped
                            if (command_data[4:0] < MAX_PATTERN_COUNT)
                                buffer_state[buffer_toggle][111:32] = pattern_table[command_data[4:0]];
                        end
                        3'b010: buffer_state[buffer_toggle][29:20] = command_data[9:0];  // X position
                        3'b011: buffer_state[buffer_toggle][19:10] = command_data[9:0];  // Y position
                        3'b100: buffer_state[buffer_toggle][9:0] = command_data[9:0];    // Additional attribute
                    endcase
                end
            end
        endcase
    end

    // Compute RGB output based on buffer status and valid addresses
    always_comb begin
        buffer_color_output[0] = (buffer_address_output[0] < ADDRESS_LIMIT) ? 
            (buffer_address_output[0][0] ? 
                color_plate[pixel_data[buffer_address_output[0][15:1]][3:2]] :
                color_plate[pixel_data[buffer_address_output[0][15:1]][1:0]]
            ) :
            color_plate[pixel_data[0]];
            
        buffer_color_output[1] = (buffer_address_output[1] < ADDRESS_LIMIT) ? 
            (buffer_address_output[1][0] ? 
                color_plate[pixel_data[buffer_address_output[1][15:1]][3:2]] :
                color_plate[pixel_data[buffer_address_output[1][15:1]][1:0]]
            ) :
            color_plate[pixel_data[0]];
                                    
        RGB_output = buffer_valid[buffer_select] ? buffer_color_output[buffer_select] : 24'h202020;
    end

    // Load pixel data from memory
    initial begin
        $readmemh("/user/stud/fall21/bk2746/Projects/EmbeddedLab/Project_hw/on_chip_mem/Mario_2bit.txt", pixel_data);
    end

endmodule
