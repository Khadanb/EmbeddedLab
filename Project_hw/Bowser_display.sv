module Bowser_display (
    input logic        clk,
    input logic        reset,
    input logic [31:0] writedata,
    input logic [9:0]  hcount,
    input logic [9:0]  vcount,
    output logic [23:0] RGB_output
);

    parameter [5:0] COMPONENT_ID = 6'b001001; // Unique ID for Bowser
    logic [3:0] mem [0:4095]; // Memory for color indices
    logic [23:0] color_palette [0:4];
    logic [79:0] pattern_table [0:0]; // Only one pattern in use

    // Setup color palette
    assign color_palette[0] = 24'hFFCC66; // Light Brown
    assign color_palette[1] = 24'h33CC33; // Green
    assign color_palette[2] = 24'hFFFFFF; // White
    assign color_palette[3] = 24'h000000; // Black
    assign color_palette[4] = 24'h202022; // Dark Gray

    // Pattern definition
    assign pattern_table[0] = {16'd0, 16'd64, 16'd64, 16'd64, 16'd64}; // Append, Res H, Res V, Act H, Act V

    // Buffers for double buffering
    logic [23:0] buffer_color_output[0:1];
    logic [15:0] buffer_address_output[0:1];
    logic buffer_valid[0:1];
    logic [111:0] buffer_state[0:1];
    logic buffer_select = 1'b0;

    // Decode writedata fields
    logic [5:0] component;
    logic [4:0] child_component;
    logic [3:0] action;
    logic [2:0] action_type;
    logic [12:0] action_data;
    logic buffer_toggle;

    assign component = writedata[31:26];
    assign child_component = writedata[25:21]; // Unused in this case
    assign action = writedata[20:17];
    assign action_type = writedata[16:14];
    assign buffer_toggle = writedata[13];
    assign action_data = writedata[12:0];

    // Address calculators for each buffer
    addr_cal addr_cal_ping(
        .pattern_info(buffer_state[0][111:32]),
        .sprite_info(buffer_state[0][31:0]),
        .hcount(hcount),
        .vcount(vcount),
        .addr_output(buffer_address_output[0]),
        .valid(buffer_valid[0])
    );

    addr_cal addr_cal_pong(
        .pattern_info(buffer_state[1][111:32]),
        .sprite_info(buffer_state[1][31:0]),
        .hcount(hcount),
        .vcount(vcount),
        .addr_output(buffer_address_output[1]),
        .valid(buffer_valid[1])
    );

    // Process input messages to control sprite parameters
    always_ff @(posedge clk) begin
        if (reset) begin
				buffer_select = buffer_toggle;
				buffer_state[~buffer_toggle][31] = 1'b0;
        end else if (component == COMPONENT_ID) begin
            case (action)
                4'b1111: begin  // Reset and toggle buffer
                    buffer_select <= buffer_toggle;
                    buffer_state[~buffer_toggle] <= 0; // Clear inactive buffer
                end
                4'h0001: begin  // Update buffer state based on input type
                    case (action_type)
                        3'b001: begin  // Set visibility and flip state
                            buffer_state[buffer_toggle][31:30] <= {action_data[12], action_data[11]};
                            buffer_state[buffer_toggle][111:32] <= pattern_table[0]; // Only one pattern
                        end
                        3'b010: begin  // Set X position
                            buffer_state[buffer_toggle][29:20] <= action_data[9:0];
                        end
                        3'b011: begin  // Set Y position
                            buffer_state[buffer_toggle][19:10] <= action_data[9:0];
                        end
                        3'b100: begin  // Additional attributes (if any)
                            buffer_state[buffer_toggle][9:0] <= action_data[9:0];
                        end
                    endcase
                end
            endcase
        end
    end

    // Determine RGB output based on active buffer state and validity
    always_comb begin
        RGB_output = buffer_valid[buffer_select] ? color_palette[mem[buffer_address_output[buffer_select]]] : 24'h202020; // Default to background color
    end

    // Initialize pixel data from memory
    initial begin
        $readmemh("/user/stud/fall21/bk2746/Projects/EmbeddedLab/Project_hw/on_chip_mem/Bowser.txt", mem);
    end
endmodule
