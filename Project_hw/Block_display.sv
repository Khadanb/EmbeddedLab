/*
 * Avalon memory-mapped peripheral that generates VGA
 * Stephen A. Edwards
 * Columbia University
 */

module Block_display (
    input logic        clk,
    input logic        reset,
    input logic [31:0] writedata,
    input logic [9:0]  hcount,
    input logic [9:0]  vcount,
    output logic [23:0] RGB_output
);

    // Configuration parameters
    parameter [5:0] COMPONENT_ID = 6'b000010;
    parameter [4:0] MAX_PATTERN_COUNT = 5'd_17;
    parameter [15:0] ADDRESS_LIMIT = 16'd_2304;
    parameter [4:0] MAX_CHILD_COMPONENTS = 5'd_9;
    logic [3:0] pixel_memory [0:2303];
    logic [23:0] color_plate [0:9];
    logic [79:0] pattern_table [0:16]; 


    // Define pattern and color data
    assign pattern_table[0] = {16'd_0, 16'd_16, 16'd_16, 16'd_16, 16'd_16};
    assign pattern_table[1] = {16'd_256, 16'd_16, 16'd_16, 16'd_16, 16'd_16};
    assign pattern_table[2] = {16'd_512, 16'd_16, 16'd_16, 16'd_16, 16'd_16};
    assign pattern_table[3] = {16'd_768, 16'd_16, 16'd_16, 16'd_16, 16'd_16};
    assign pattern_table[4] = {16'd_1024, 16'd_16, 16'd_16, 16'd_16, 16'd_16};
    assign pattern_table[5] = {16'd_1280, 16'd_16, 16'd_16, 16'd_16, 16'd_16};
    assign pattern_table[6] = {16'd_1536, 16'd_16, 16'd_16, 16'd_16, 16'd_16};
    assign pattern_table[7] = {16'd_1792, 16'd_16, 16'd_16, 16'd_16, 16'd_16};
    assign pattern_table[8] = {16'd_2048, 16'd_16, 16'd_16, 16'd_16, 16'd_16};
    assign pattern_table[9] = {16'd_1280, 16'd_16, 16'd_16, 16'd_32, 16'd_16};
    assign pattern_table[10] = {16'd_1280, 16'd_16, 16'd_16, 16'd_48, 16'd_16};
    assign pattern_table[11] = {16'd_1280, 16'd_16, 16'd_16, 16'd_128, 16'd_16};
    assign pattern_table[12] = {16'd_2048, 16'd_16, 16'd_16, 16'd_16, 16'd_32};
    assign pattern_table[13] = {16'd_2048, 16'd_16, 16'd_16, 16'd_16, 16'd_48};
    assign pattern_table[14] = {16'd_2048, 16'd_16, 16'd_16, 16'd_16, 16'd_64};
    assign pattern_table[15] = {16'd_2048, 16'd_16, 16'd_16, 16'd_64, 16'd_64};
	assign pattern_table[16] = {16'd_2048, 16'd_16, 16'd_16, 16'd_16, 16'd_64};

	assign color_plate[0] = 24'h202020;
	assign color_plate[1] = 24'h908fff;
	assign color_plate[2] = 24'h9a4b00;
	assign color_plate[3] = 24'he69a25;
	assign color_plate[4] = 24'h000000;
	assign color_plate[5] = 24'h974f00;
	assign color_plate[6] = 24'h572200;
	assign color_plate[7] = 24'h1a9300;
	assign color_plate[8] = 24'hfce1ce;
	assign color_plate[9] = 24'hffcdc4;

    // Buffers for rendering logic
    logic [23:0] buffer_color_output[2][9];
    logic [15:0] buffer_address_output[2][9];
    logic        buffer_valid[2][9];
    logic [111:0] buffer_state[2][9];
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
        for (i = 0; i < MAX_CHILD_COMPONENTS; i++) begin : gen_addr_cal
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

    // Manage input actions and update state
    always_ff @(posedge clk) begin
        case (action)
            4'b1111: begin  // Reset buffers based on toggle
                buffer_select = buffer_toggle;
                for (int j = 0; j < MAX_CHILD_COMPONENTS; j++) begin
                    buffer_state[~buffer_toggle][j][31] = 1'b0;  // Clear visibility
                end
            end
            4'h0001: if (component == COMPONENT_ID && child_component < MAX_CHILD_COMPONENTS) begin
                // Update specific child component settings
                case (action_type)
                    3'b001: begin  // Visibility and pattern
                        buffer_state[buffer_toggle][child_component][31] = action_data[12];
                        buffer_state[buffer_toggle][child_component][30] = action_data[11];
                        if (action_data[4:0] < MAX_PATTERN_COUNT) begin
                            buffer_state[buffer_toggle][child_component][111:32] = pattern_table[action_data[4:0]];
                        end
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
        for (int k = 0; k < MAX_CHILD_COMPONENTS; k++) begin
            if (buffer_valid[buffer_select][k]) begin
                RGB_output = color_plate[pixel_memory[buffer_color_output[buffer_select][k]]];
                break;
            end
        end
    end


    // Initialize pixel data from memory
    initial begin
        $readmemh("/user/stud/fall21/bk2746/Projects/EmbeddedLab/Project_hw/on_chip_mem/Block.txt", pixel_memory);
    end

endmodule
