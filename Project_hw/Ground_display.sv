/*
 * Avalon memory-mapped peripheral that generates VGA
 *
 * Stephen A. Edwards
 * Columbia University
 */

module Ground_display (
    input logic        clk,
    input logic        reset,
    input logic [31:0] writedata,
    input logic [9:0]  hcount,
    input logic [9:0]  vcount,
    output logic [23:0] RGB_output
);

    // Parameters for configuration
    parameter [5:0] COMPONENT_ID = 6'b001111;  // Unique identifier for this component
    parameter [4:0] NUM_PATTERNS = 5'd_1;        // Number of different patterns
    parameter [15:0] ADDRESS_LIMIT = 16'd_256;   // Memory address limit for color indexing
    logic [3:0] pixel_memory [0:127];           // Memory for pixel data
    logic [23:0] color_palette [0:3];           // Palette of colors available
    logic [79:0] pattern_table [0:1];           // Table storing pattern information

    assign pattern_table[0] = {16'd_0, 16'd_16, 16'd_16, 16'd_650, 16'd_32};

    // Color palette definitions
    assign color_palette[0] = 24'h202020;  // Background color
    assign color_palette[1] = 24'hFFFFFF;  // White
    assign color_palette[2] = 24'h808080;  // Gray
    assign color_palette[3] = 24'hD3D3D3;  // Light Gray

    parameter [9:0] GROUND_LEVEL = 10'd_368;  // Vertical position of the ground

    // Ping-pong buffer outputs and states
    logic [23:0] buffer_color_output[0:1];
    logic [15:0] buffer_address_output[0:1];
    logic        buffer_address_valid[0:1];
    logic [111:0] buffer_state[0:1];
    logic        buffer_select = 1'b0;

    // Decoded writedata fields
    logic [5:0] component;
    logic [4:0] subcomponent;
    logic [3:0] action;
    logic [2:0] action_type;
    logic [12:0] message;
    logic        buffer_toggle;

    // Decoding writedata
    assign component = writedata[31:26];
    assign subcomponent = writedata[25:21];
    assign action = writedata[20:17];
    assign action_type = writedata[16:14];
    assign buffer_toggle = writedata[13];
    assign message = writedata[12:0];

    // Assign static ground level to buffer states
    assign buffer_state[0][19:10] = GROUND_LEVEL;
    assign buffer_state[1][19:10] = GROUND_LEVEL;
    logic [9:0] left_edge = 10'd0;
    logic [9:0] right_edge = 10'd0;

    // Address calculation blocks for each buffer
    addr_cal address_calculator_ping(
        .pattern_info(buffer_state[0][111:32]),
        .sprite_info(buffer_state[0][31:0]),
        .hcount(hcount),
        .vcount(vcount),
        .addr_output(buffer_address_output[0]),
        .valid(buffer_address_valid[0])
    );

    addr_cal address_calculator_pong(
        .pattern_info(buffer_state[1][111:32]),
        .sprite_info(buffer_state[1][31:0]),
        .hcount(hcount),
        .vcount(vcount),
        .addr_output(buffer_address_output[1]),
        .valid(buffer_address_valid[1])
    );

    // Processing inputs
    always_ff @(posedge clk) begin
        case (action)
            4'b1111: begin  // Flush and clear buffers based on toggle
                buffer_select = buffer_toggle;
                buffer_state[~buffer_toggle][31] = 1'b0;
            end
            4'h0001 : begin  // Write to state holder
                if (component == COMPONENT_ID) begin
                    case (action_type)
                        3'b001: begin  // Update visibility and pattern
                            buffer_state[buffer_toggle][31] = message[12];  // Visible
                            buffer_state[buffer_toggle][30] = message[11];  // Flipped
                            if (message[4:0] < NUM_PATTERNS)
                                buffer_state[buffer_toggle][111:32] = pattern_table[message[4:0]];
                        end
                        3'b010: buffer_state[buffer_toggle][29:20] = message[9:0];  // X coordinate
                        3'b011: left_edge = message[9:0];  // Y start coordinate
                        3'b100: right_edge = message[9:0];  // Y end coordinate
                    endcase
                end
            end
        endcase
    end

    // Color output determination
    assign buffer_color_output[0] = (buffer_address_output[0] < ADDRESS_LIMIT) ?
        (buffer_address_output[0][0] ? 
            color_palette[pixel_memory[buffer_address_output[0][15:1]][3:2]] :
            color_palette[pixel_memory[buffer_address_output[0][15:1]][1:0]]) :
        color_palette[pixel_memory[0]];

    assign buffer_color_output[1] = (buffer_address_output[1] < ADDRESS_LIMIT) ?
        (buffer_address_output[1][0] ? 
            color_palette[pixel_memory[buffer_address_output[1][15:1]][3:2]] :
            color_palette[pixel_memory[buffer_address_output[1][15:1]][1:0]]) :
        color_palette[pixel_memory[0]];

    // Determine final RGB output
    assign RGB_output = buffer_address_valid[buffer_select] ? 
        (((hcount < left_edge) || (hcount > right_edge)) ? buffer_color_output[buffer_select] : 24'h202020) :
        24'h202020;

    // Initialize pixel memory
    initial begin
        $readmemh("/user/stud/fall21/bk2746/Projects/EmbeddedLab/Project_hw/on_chip_mem/Ground_2bit.txt", pixel_memory);
    end

endmodule
