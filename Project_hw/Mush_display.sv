module Mush_display (
    input logic        clk,
    input logic        reset,
    input logic [31:0]  writedata,
    input logic [9:0]   hcount,
    input logic [9:0]   vcount,
    output logic [23:0] RGB_output
);

    // Configuration parameters
    parameter [5:0] DEVICE_ID = 6'b001001;
    parameter [4:0] MAX_PATTERNS = 5'd2;
    parameter [15:0] ADDRESS_LIMIT = 16'd512;
    parameter [4:0] MAX_SPRITES = 5'd2;

    // Memory and colors
    logic [3:0] sprite_memory [0:255];
    logic [23:0] color_palette [0:3];
    logic [79:0] pattern_data [0:1];

    // Initialize color palette and pattern data
    initial begin
        color_palette[0] = 24'h9290ff;
        color_palette[1] = 24'he69c21;
        color_palette[2] = 24'h0c9300;
        color_palette[3] = 24'hfffeff;
        pattern_data[0] = {16'd_0, 16'd_16, 16'd_16, 16'd_16, 16'd_16};
        pattern_data[1] = {16'd_256, 16'd_16, 16'd_16, 16'd_16, 16'd_16};
    end

    // Double buffering
    logic [23:0] double_buffer_color[2][2];
    logic [15:0] double_buffer_addr[2][2];
    logic double_buffer_valid[2][2];
    logic [111:0] sprite_state[2][2];
    logic current_buffer = 1'b0;

    // Decode writedata fields
    logic [5:0] component_id;
    logic [4:0] sprite_id;
    logic [3:0] command;
    logic [2:0] input_type;
    logic [12:0] input_data;
    logic buffer_select;

    assign component_id = writedata[31:26];
    assign sprite_id = writedata[25:21];
    assign command = writedata[20:17];
    assign input_type = writedata[16:14];
    assign buffer_select = writedata[13];
    assign input_data = writedata[12:0];

    // Address calculators for each buffer
    genvar gi, gj;
    generate
        for (gi = 0; gi < 2; gi++) begin : buffers
            for (gj = 0; gj < 2; gj++) begin : sprites
                addr_cal addr_cal_inst(
                    .pattern_info(sprite_state[gi][gj][111:32]),
                    .sprite_info(sprite_state[gi][gj][31:0]),
                    .hcount(hcount),
                    .vcount(vcount),
                    .addr_output(double_buffer_addr[gi][gj]),
                    .valid(double_buffer_valid[gi][gj])
                );
            end
        end
    endgenerate

    // Manage input data and update sprite states
    always_ff @(posedge clk) begin
        if (reset) begin
            // Reset logic can be added here if necessary
        end else begin
            case (command)
                4'b1111: begin  // Flush and clear buffers
                    current_buffer = buffer_select;
                    for (int i = 0; i < MAX_SPRITES; i++) begin
                        sprite_state[~current_buffer][i][31] = 1'b0;
                    end
                end
                4'h1: if (component_id == DEVICE_ID && sprite_id < MAX_SPRITES) begin
                    sprite_state[current_buffer][sprite_id][31] = input_data[12];  // Visibility
                    sprite_state[current_buffer][sprite_id][30] = input_data[11];  // Flip
                    if (input_type == 3'b001 && input_data[4:0] < MAX_PATTERNS) begin
                        sprite_state[current_buffer][sprite_id][111:32] = pattern_data[input_data[4:0]];
                    end
                    sprite_state[current_buffer][sprite_id][29:20] = input_data[9:0];  // X position
                    sprite_state[current_buffer][sprite_id][19:10] = input_data[9:0];  // Y position
                    sprite_state[current_buffer][sprite_id][9:0] = input_data[9:0];    // Additional attributes
                end
            endcase
        end
    end

    // Calculate the RGB output
    always_comb begin
        RGB_output = 24'h9290ff; // Default to background color
        for (int j = 0; j < MAX_SPRITES; j++) begin
            if (double_buffer_valid[current_buffer][j]) begin
                logic [15:0] addr = double_buffer_addr[current_buffer][j];
                RGB_output = (addr < ADDRESS_LIMIT) ? color_palette[sprite_memory[addr >> 1][1:0]] : color_palette[0];
                if (RGB_output != 24'h9290ff) break;
            end
        end
    end

    // Initialize sprite memory
    initial $readmemh("/user/stud/fall21/bk2746/Projects/EmbeddedLab/Project_hw/on_chip_mem/Mush_2bit.txt", sprite_memory);
endmodule
