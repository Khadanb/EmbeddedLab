module Bowser_display (
    input logic        clk,
    input logic        reset,
    input logic [31:0] writedata,
    input logic [9:0]  hcount,
    input logic [9:0]  vcount,
    output logic [23:0] RGB_output
);

    // Configuration parameters
    parameter [5:0] COMPONENT_ID = 6'b001001; // Unique ID for Bowser
    parameter [4:0] pattern_num = 5'd1;
    parameter [15:0] addr_limit = 16'd4096; // Adjusted for memory size
    logic [1:0] mem [0:4095]; // Adjusted memory indexing for 2-bit color
    logic [23:0] color_plate [0:4];

    // Color plate assignments
    assign color_plate[0] = 24'hFFCC66;  // Light Brown
    assign color_plate[1] = 24'h33CC33;  // Green
    assign color_plate[2] = 24'hFFFFFF;  // White
    assign color_plate[3] = 24'h000000;  // Black
    assign color_plate[4] = 24'h202022;  // Dark Gray

    // Buffer state and output
    logic [23:0] buffer_RGB_output[0:1];
    logic [15:0] buffer_addr_output[0:1];
    logic buffer_addr_valid[0:1];
    logic [111:0] buffer_state[0:1];
    logic buffer_select = 1'b0;

    // Decode writedata
    logic [5:0] sub_comp;
    logic [4:0] child_comp;
    logic [3:0] info;
    logic [2:0] input_type;
    logic [12:0] input_msg;
    logic select;

    assign sub_comp = writedata[31:26];
    assign child_comp = writedata[25:21];
    assign info = writedata[20:17];
    assign input_type = writedata[16:14];
    assign select = writedata[13];
    assign input_msg = writedata[12:0];

    // Address calculators for each buffer
    addr_cal addr_cal_ping(
        .pattern_info(buffer_state[0][111:32]),
        .sprite_info(buffer_state[0][31:0]),
        .hcount(hcount),
        .vcount(vcount),
        .addr_output(buffer_addr_output[0]),
        .valid(buffer_addr_valid[0])
    );

    addr_cal addr_cal_pong(
        .pattern_info(buffer_state[1][111:32]),
        .sprite_info(buffer_state[1][31:0]),
        .hcount(hcount),
        .vcount(vcount),
        .addr_output(buffer_addr_output[1]),
        .valid(buffer_addr_valid[1])
    );

    // Handle writedata inputs and buffer switching
    always_ff @(posedge clk) begin
        if (reset) begin
            buffer_select <= 0;
            buffer_state[0] <= 0;
            buffer_state[1] <= 0;
        end else begin
            if (sub_comp == COMPONENT_ID) begin
                case (info)
                    4'b1111: begin  // Reset and toggle buffer
                        buffer_select <= select;
                        buffer_state[~select] <= 0; // Clear inactive buffer
                    end
                    4'h0001: begin  // Update buffer state based on input
                        if (input_type == 3'b001) begin
                            buffer_state[select][111:0] <= {16'd0, 16'd64, 16'd64, writedata[25:16], writedata[15:6]};
                        end
                    end
                endcase
            end
        end
    end

    // Select RGB output based on buffer validity
    always_comb begin
        if (buffer_addr_valid[buffer_select]) begin
            RGB_output = color_plate[mem[buffer_addr_output[buffer_select]]];
        end else begin
            RGB_output = 24'h202020; // Default background color
        end
    end

    // Initialize memory
    initial begin
        $readmemh("/user/stud/fall21/bk2746/Projects/EmbeddedLab/Project_hw/on_chip_mem/Bowser.txt", mem);
    end
endmodule
