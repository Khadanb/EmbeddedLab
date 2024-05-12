module Peach_display (
    input logic        clk,
    input logic        reset,
    input logic [31:0] writedata,
    input logic [9:0]  hcount,
    input logic [9:0]  vcount,
    output logic [23:0] RGB_output
);

    parameter [5:0] COMPONENT_ID = 6'b001010; 
    parameter [15:0] addr_limit = 16'd3001;
    logic [3:0] mem [0:3000]; 
    logic [23:0] color_palette [0:4];
    logic [79:0] pattern_table [0:0]; 

    
    assign color_palette[0] = 24'hFF0000; 
    assign color_palette[1] = 24'hFFCC66; 
    assign color_palette[2] = 24'hFFFFFF; 
    assign color_palette[3] = 24'h202020; 

    
    assign pattern_table[0] = {16'd0, 16'd32, 16'd40, 16'd32, 16'd40}; 

    
    logic [23:0] buffer_color_output[0:1];
    logic [15:0] buffer_address_output[0:1];
    logic buffer_valid[0:1];
    logic [111:0] buffer_state[0:1];
    logic buffer_select = 1'b0;

    
    logic [5:0] component;
    logic [3:0] action;
    logic [2:0] action_type;
    logic [12:0] action_data;
    logic buffer_toggle;

    assign component = writedata[31:26];
    assign action = writedata[20:17];
    assign action_type = writedata[16:14];
    assign buffer_toggle = writedata[13];
    assign action_data = writedata[12:0];

    
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

    
    always_ff @(posedge clk) begin
        if (reset) begin
            buffer_select <= 0;
            buffer_state[0] <= 0;
            buffer_state[1] <= 0;
        end else begin
            case (action)
                4'b1111: begin  
                    buffer_select <= buffer_toggle;
                    buffer_state[~buffer_toggle] <= 1'b0; 
                end
                4'h0001: if (component == COMPONENT_ID) begin
                    
                    case (action_type)
                        3'b001: begin  
                            buffer_state[buffer_toggle][31:30] <= {action_data[12], action_data[11]};
                            if (action_data[4:0] == 0) 
                                buffer_state[buffer_toggle][111:32] <= pattern_table[action_data[4:0]];;
                        end
                        3'b010: buffer_state[buffer_toggle][29:20] <= action_data[9:0]; 
                        3'b011: buffer_state[buffer_toggle][19:10] <= action_data[9:0]; 
                        3'b100: buffer_state[buffer_toggle][9:0] <= action_data[9:0]; 
                    endcase
                end
            endcase
        end
    end

    
    always_comb begin
        if (buffer_valid[buffer_select] && buffer_address_output[buffer_select] < addr_limit) begin
            RGB_output = color_palette[mem[buffer_address_output[buffer_select]]]; 
        end else begin
            RGB_output = 24'h202020; 
        end
    end

    
    initial begin
        $readmemh("/user/stud/fall21/bk2746/Projects/EmbeddedLab/Project_hw/on_chip_mem/Peach.txt", mem);
    end
endmodule
