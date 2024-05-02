/*
 * Avalon memory-mapped peripheral that generates VGA
 * Stephen A. Edwards
 * Columbia University
 */

module Cloud_display (
    input logic        clk,
    input logic        reset,
    input logic [31:0]  writedata,
    input logic [9:0]   hcount,
    input logic [9:0]   vcount,
    output logic [23:0] RGB_output
);

    // Constants for module configuration
    parameter [5:0] COMPONENT_ID = 6'b001110; 
    parameter [4:0] MAX_PATTERN_INDEX = 5'd_1;
    parameter [15:0] ADDRESS_LIMIT = 16'd_768;
    parameter [4:0] MAX_CHILDREN = 5'd_1;

    // Memory and color configuration
    logic [3:0] memory [0:383];
    logic [23:0] color_palette [0:3] = {24'h202020, 24'h000000, 24'hffffff, 24'h63adff};
    logic [79:0] pattern_table [0:2] = {{16'd_0, 16'd_32, 16'd_24, 16'd_32, 16'd_24}};

    // Double buffering variables
    logic [23:0] buffer_rgb_output[2];
    logic [15:0] buffer_address_output[2];
    logic buffer_valid[2];
    logic [111:0] buffer_state_data[2];
    logic buffer_select = 1'b0;

    // Extract writedata fields
    logic [5:0] component;
    logic [4:0] child_index;
    logic [3:0] info;
    logic [2:0] data_type;
    logic [12:0] message_data;
    logic buffer_to_activate;

    assign component = writedata[31:26];
    assign child_index = writedata[25:21];
    assign info = writedata[20:17];
    assign data_type = writedata[16:14];
    assign buffer_to_activate = writedata[13];
    assign message_data = writedata[12:0];

    // Address calculators for each buffer
    addr_cal addr_cal_ping(
        .pattern_info(buffer_state_data[0][111:32]), 
        .sprite_info(buffer_state_data[0][31:0]), 
        .hcount(hcount), 
        .vcount(vcount), 
        .addr_output(buffer_address_output[0]), 
        .valid(buffer_valid[0])
    );

    addr_cal addr_cal_pong(
        .pattern_info(buffer_state_data[1][111:32]), 
        .sprite_info(buffer_state_data[1][31:0]), 
        .hcount(hcount), 
        .vcount(vcount), 
        .addr_output(buffer_address_output[1]), 
        .valid(buffer_valid[1])
    );

    // Handle buffer operations and sprite updates
    always_ff @(posedge clk) begin
        case (info)
            4'b1111: begin // Flush and switch buffer
                buffer_select = buffer_to_activate;
                buffer_state_data[~buffer_select][31] = 1'b0; // Clear visibility of the non-selected buffer
            end
            4'h1: if (component == COMPONENT_ID) begin // Update sprite states
                buffer_state_data[buffer_select][31] = message_data[12]; // visibility
                buffer_state_data[buffer_select][30] = message_data[11]; // flip status
                if (data_type == 3'b001 && message_data[4:0] < MAX_PATTERN_INDEX) begin
                    buffer_state_data[buffer_select][111:32] = pattern_table[message_data[4:0]];
                end
                buffer_state_data[buffer_select][29:20] = message_data[9:0]; // x coordinate
                buffer_state_data[buffer_select][19:10] = message_data[9:0]; // y coordinate
                buffer_state_data[buffer_select][9:0] = message_data[9:0]; // shift amount
            end
        endcase
    end

    // Calculate RGB output based on buffer state and memory contents
    always_comb begin
        for (int i = 0; i < 2; i++) begin
            buffer_rgb_output[i] = (buffer_address_output[i] < ADDRESS_LIMIT) ? 
                color_palette[memory[buffer_address_output[i] >> 1][1:0]] : 
                color_palette[0];
        end
        RGB_output = buffer_valid[buffer_select] ? buffer_rgb_output[buffer_select] : 24'h202020;
    end

    // Initialize memory from a file
    initial begin
        $readmemh("/user/stud/fall21/bk2746/Projects/EmbeddedLab/Project_hw/on_chip_mem/Cloud_2bit.txt", memory);
    end

endmodule
