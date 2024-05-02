module Coin_display (
    input logic        clk,
    input logic        reset,
    input logic [31:0]  writedata,
    input logic [9:0]   hcount,
    input logic [9:0]   vcount,
    output logic [23:0] RGB_output
);

    // Configuration parameters
    parameter [5:0] COMPONENT_ID = 6'b000011;
    parameter [4:0] MAX_PATTERN_INDEX = 5'd4;
    parameter [15:0] ADDRESS_LIMIT = 16'd512;
    parameter [4:0] MAX_CHILDREN = 5'd4;
    
    // Memory and color definitions
    logic [3:0] sprite_memory [0:511];
    logic [23:0] color_plate [0:4];
    logic [79:0] pattern_table [0:3];

    // Initialize pattern and color data
    initial begin
        color_plate[0] = 24'h202020;
        color_plate[1] = 24'h908fff;
        color_plate[2] = 24'he59a25;
        color_plate[3] = 24'hfffeff;
        color_plate[4] = 24'hb33425;

        pattern_table[0] = {16'd0, 16'd8, 16'd16, 16'd8, 16'd16};
        pattern_table[1] = {16'd128, 16'd8, 16'd16, 16'd8, 16'd16};
        pattern_table[2] = {16'd256, 16'd8, 16'd16, 16'd8, 16'd16};
        pattern_table[3] = {16'd384, 16'd8, 16'd16, 16'd8, 16'd16};
    end

    // Buffer definitions for double-buffering
    logic [23:0] buffer_color_output[2][MAX_CHILDREN];
    logic [15:0] buffer_address_output[2][MAX_CHILDREN];
    logic buffer_valid[2][MAX_CHILDREN];
    logic [111:0] buffer_state_data[2][MAX_CHILDREN];
    logic active_buffer = 1'b0;

    // Extract fields from writedata
    logic [5:0] component_id;
    logic [4:0] child_index;
    logic [3:0] control_code;
    logic [2:0] data_type;
    logic [12:0] message_data;
    logic selected_buffer;

    // Decode input writedata
    assign component_id = writedata[31:26];
    assign child_index = writedata[25:21];
    assign control_code = writedata[20:17];
    assign data_type = writedata[16:14];
    assign selected_buffer = writedata[13];
    assign message_data = writedata[12:0];

    // Address calculation instances
    genvar i;
    generate
        for (i = 0; i < MAX_CHILDREN; i++) begin : buffer_processing
            addr_cal addr_cal_instance(
                .pattern_info(buffer_state_data[0][i][111:32]),
                .sprite_info(buffer_state_data[0][i][31:0]),
                .hcount(hcount),
                .vcount(vcount),
                .addr_output(buffer_address_output[0][i]),
                .valid(buffer_valid[0][i])
            );
            addr_cal addr_cal_instance_pong(
                .pattern_info(buffer_state_data[1][i][111:32]),
                .sprite_info(buffer_state_data[1][i][31:0]),
                .hcount(hcount),
                .vcount(vcount),
                .addr_output(buffer_address_output[1][i]),
                .valid(buffer_valid[1][i])
            );
        end
    endgenerate

    // Handle input data and buffer updates
    always_ff @(posedge clk) begin
        case (control_code)
            4'b1111: begin // Buffer switch and clear
                active_buffer = selected_buffer;
                for (int idx = 0; idx < MAX_CHILDREN; idx++) begin
                    buffer_state_data[~active_buffer][idx][31] = 1'b0; // Clear visibility
                end
            end
            4'h1: if (component_id == COMPONENT_ID && child_index < MAX_CHILDREN) begin
                buffer_state_data[selected_buffer][child_index][31:30] = message_data[12:11]; // visibility and flip
                if (data_type == 3'b001 && message_data[4:0] < MAX_PATTERN_INDEX) begin
                    buffer_state_data[selected_buffer][child_index][111:32] = pattern_table[message_data[4:0]];
                end
                buffer_state_data[selected_buffer][child_index][29:0] = message_data[9:0]; // coordinates and shift
            end
        endcase
    end

    // Determine the output color
    always_comb begin
        RGB_output = 24'h202020; // Default color
        for (int idx = 0; idx < MAX_CHILDREN; idx++) begin
            if (buffer_valid[active_buffer][idx]) begin
                buffer_color_output[active_buffer][idx] = (buffer_address_output[active_buffer][idx] < ADDRESS_LIMIT) ? 
                    color_plate[sprite_memory[buffer_address_output[active_buffer][idx] >> 1][1:0]] : color_plate[0];
                RGB_output = buffer_color_output[active_buffer][idx];
                break; // Use the first valid output
            end
        end
    end

    // Initialize memory
    initial $readmemh("/user/stud/fall21/bk2746/Projects/EmbeddedLab/Project_hw/on_chip_mem/Coin.txt", sprite_memory);

endmodule
