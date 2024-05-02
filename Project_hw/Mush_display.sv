module Mush_display (
    input logic        clk,
    input logic        reset,
    input logic [31:0] writedata,
    input logic [9:0]  hcount,
    input logic [9:0]  vcount,
    output logic [23:0] RGB_output
);

    // Constants
    parameter [5:0] COMPONENT_ID = 6'b001001;
    parameter [4:0] NUM_PATTERNS = 5'd_2;
    parameter [15:0] ADDR_LIMIT = 16'd_512;
    parameter [4:0] MAX_CHILDREN = 5'd_2;

    // Memory storage
    logic [3:0] mem [0:255];
    logic [23:0] color_plate [0:3];
    logic [79:0] pattern_table [0:1];

    // Initialize color_plate and pattern_table
    initial begin
        color_plate[0] = 24'h202020;
        color_plate[1] = 24'he69c21;
        color_plate[2] = 24'h0c9300;
        color_plate[3] = 24'hfffeff;

        pattern_table[0] = {16'd_0, 16'd_16, 16'd_16, 16'd_16, 16'd_16};
        pattern_table[1] = {16'd_256, 16'd_16, 16'd_16, 16'd_16, 16'd_16};
    end

    // Ping-pong buffering for double-buffering technique
    logic [23:0] buffered_color_output[2][2];
    logic [15:0] buffered_address_output[2][2];
    logic buffer_valid[2][2];
    logic [111:0] buffered_state_data[2][2];
    logic buffer_select = 1'b0;
    logic [15:0] addr;

    // Data extraction from writedata
    logic [5:0] component;
    logic [4:0] child_index;
    logic [3:0] control_code;
    logic [2:0] data_type;
    logic [12:0] payload;
    logic select_buffer;

    assign component = writedata[31:26];
    assign child_index = writedata[25:21];
    assign control_code = writedata[20:17];
    assign data_type = writedata[16:14];
    assign select_buffer = writedata[13];
    assign payload = writedata[12:0];

    // Address calculators for rendering logic
    genvar gi, gj;
    generate
        for (gi = 0; gi < 2; gi++) begin : buffer_generation
            for (gj = 0; gj < 2; gj++) begin : child_generation
                addr_cal addr_calc_inst(
                    .pattern_info(buffered_state_data[gi][gj][111:32]),
                    .sprite_info(buffered_state_data[gi][gj][31:0]),
                    .hcount(hcount),
                    .vcount(vcount),
                    .addr_output(buffered_address_output[gi][gj]),
                    .valid(buffer_valid[gi][gj])
                );
            end
        end
    endgenerate

    // Buffer control and data updating
    always_ff @(posedge clk) begin
        case (control_code)
            4'b1111: begin // Flush and clear buffer
                buffer_select = select_buffer;
                for (int i = 0; i < MAX_CHILDREN; i++) begin
                    buffered_state_data[~select_buffer][i][31] = 1'b0; // Clear visibility
                end
            end
            4'h1: if (component == COMPONENT_ID && child_index < MAX_CHILDREN) begin
                // Update state based on data type
                buffered_state_data[select_buffer][child_index][31:30] = payload[12:11]; // visibility and flip
                if (data_type == 3'b001 && payload[4:0] < NUM_PATTERNS)
                    buffered_state_data[select_buffer][child_index][111:32] = pattern_table[payload[4:0]]; // pattern
                buffered_state_data[select_buffer][child_index][29:10] = payload[9:0]; // coordinates and shift
            end
        endcase
    end

// Output computation
    always_comb begin
        RGB_output = 24'h202020; // Default to background color
        for (int j = 0; j < MAX_CHILDREN; j++) begin
            if (buffer_valid[buffer_select][j]) begin
                addr = buffered_address_output[buffer_select][j]; // Calculate address once validated
                RGB_output = (addr < ADDR_LIMIT) ? color_plate[mem[addr >> 1]] : color_plate[0];
                if (RGB_output != color_plate[0]) break; // Use first non-default color found
            end
        end
    end

    // Initialize memory from a file
    initial $readmemh("/user/stud/fall21/bk2746/Projects/EmbeddedLab/Project_hw/on_chip_mem/Mush_2bit.txt", mem);
endmodule
