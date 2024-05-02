/*
 * Avalon memory-mapped peripheral that generates VGA
 * 
 * Stephen A. Edwards
 * Columbia University
 */

module Goomba_display (
    input logic        clk,
    input logic        reset,
    input logic [31:0] writedata,
    input logic [9:0]  hcount,
    input logic [9:0]  vcount,
    output logic [23:0] RGB_output
);

    // Constants for configuration
    parameter [5:0] COMPONENT_ID = 6'b000101;  // ID for this component
    parameter [4:0] MAX_PATTERN_COUNT = 5'd_2;  // Number of patterns supported
    parameter [15:0] ADDRESS_LIMIT = 16'd_384;  // Memory address limit
    parameter [4:0] CHILD_COMPONENT_LIMIT = 5'd_2;  // Limit for child components
    logic [3:0] pixel_data [0:191];  // Pixel memory array
    logic [23:0] color_palette [0:3];  // Palette of colors
    logic [79:0] pattern_data [0:1];  // Pattern data

    // Define pattern and color data
    assign pattern_data[0] = {16'd_0, 16'd_16, 16'd_16, 16'd_16, 16'd_16};
    assign pattern_data[1] = {16'd_256, 16'd_16, 16'd_8, 16'd_16, 16'd_8};

    assign color_palette[0] = 24'h202020;  // Background color
    assign color_palette[1] = 24'h9c4a00;  // Brown
    assign color_palette[2] = 24'h000000;  // Black
    assign color_palette[3] = 24'hffcec5;  // Peach

    // Buffers for rendering logic
    logic [23:0] buffer_color_output[0:1][0:1];
    logic [15:0] buffer_address_output[0:1][0:1];
    logic        buffer_valid[0:1][0:1];
    logic [111:0] buffer_state[0:1][0:1];
    logic        buffer_select = 1'b0;

    // Decode writedata fields
    logic [5:0] component;
    logic [4:0] child_component;
    logic [3:0] action;
    logic [2:0] action_type;
    logic [12:0] action_data;
    logic        buffer_toggle;

    // Decode input writedata
    assign component = writedata[31:26];
    assign child_component = writedata[25:21];
    assign action = writedata[20:17];
    assign action_type = writedata[16:14];
    assign buffer_toggle = writedata[13];
    assign action_data = writedata[12:0];

    // Address calculators for each child component in both buffer states
    genvar i;
    generate
        for (i = 0; i < CHILD_COMPONENT_LIMIT; i = i + 1) begin : gen_addr_cal
            addr_cal address_calculator_ping(
                .pattern_info(buffer_state[0][i][111:32]),
                .sprite_info(buffer_state[0][i][31:0]),
                .hcount(hcount),
                .vcount(vcount),
                .addr_output(buffer_address_output[0][i]),
                .valid(buffer_valid[0][i])
            );

            addr_cal address_calculator_pong(
                .pattern_info(buffer_state[1][i][111:32]),
                .sprite_info(buffer_state[1][i][31:0]),
                .hcount(hcount),
                .vcount(vcount),
                .addr_output(buffer_address_output[1][i]),
                .valid(buffer_valid[1][i])
            );
        end
    endgenerate

    // Processing inputs and managing state
    always_ff @(posedge clk) begin
        case (action)
            4'b1111: begin  // Reset buffers based on toggle
                buffer_select = buffer_toggle;
                for (int j = 0; j < CHILD_COMPONENT_LIMIT; j = j + 1) begin
                    buffer_state[~buffer_toggle][j][31] = 1'b0;  // Clear visibility
                end
            end
            4'h0001: if (component == COMPONENT_ID && child_component < CHILD_COMPONENT_LIMIT) begin
                // Update specific child component settings
                case (action_type)
                    3'b001: begin  // Visibility and pattern
                        buffer_state[buffer_toggle][child_component][31] = action_data[12];
                        buffer_state[buffer_toggle][child_component][30] = action_data[11];
                        if (action_data[4:0] < MAX_PATTERN_COUNT)
                            buffer_state[buffer_toggle][child_component][111:32] = pattern_data[action_data[4:0]];
                    end
                    3'b010: buffer_state[buffer_toggle][child_component][29:20] = action_data[9:0];  // X position
                    3'b011: buffer_state[buffer_toggle][child_component][19:10] = action_data[9:0];  // Y position
                    3'b100: buffer_state[buffer_toggle][child_component][9:0] = action_data[9:0];    // Additional attribute
                endcase
            end
        endcase
    end

    // Determine RGB output based on active buffer state and validity
    always_comb begin
        RGB_output = 24'h202020;  // Default to background color
        for (int k = 0; k < CHILD_COMPONENT_LIMIT; k = k + 1) begin
            if (buffer_valid[buffer_select][k]) begin
                RGB_output = buffer_color_output[buffer_select][k];
                if (RGB_output != 24'h202020) break;  // Exit loop on first valid color
            end
        end
    end

    // Initialize pixel data from memory
    initial begin
        $readmemh("/user/stud/fall21/bk2746/Projects/EmbeddedLab/Project_hw/on_chip_mem/Goomba_2bit.txt", pixel_data);
    end

endmodule
