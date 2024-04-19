/*
 * Avalon memory-mapped peripheral that generates VGA
 *
 * Stephen A. Edwards
 * Columbia University
 */

module addr_cal (
		input logic [79:0]  pattern_info,
        input logic [31:0]  sprite_info,
        input logic [9:0]   hcount,
		input logic [9:0]   vcount,

		output logic [15:0]	addr_output,
        output logic        valid);

    // sprite param
    logic [9:0]   x_pos;
	logic [9:0]   y_pos;
    logic [9:0]   shift_amount;
    logic         flip;
    logic         visible;
    
    assign visible = sprite_info[31];
    assign flip = sprite_info[30];
    assign x_pos = sprite_info[29:20];
    assign y_pos = sprite_info[19:10];
    assign shift_amount = sprite_info[9:0];

    // pattern param
    logic [15:0]    pattern_append, pattern_res_h, pattern_res_v, pattern_act_h, pattern_act_v;
    logic [15:0]    x_pos_16, y_pos_16;
    assign pattern_append = pattern_info[79:64];
    assign pattern_res_h = pattern_info[63:48];
    assign pattern_res_v = pattern_info[47:32];
    assign pattern_act_h = pattern_info[31:16];
    assign pattern_act_v = pattern_info[15:0];
    assign x_pos_16[9:0] = x_pos;
    assign y_pos_16[9:0] = y_pos;

    //Calculation param
    logic [15:0]    pattern_addr_x, pattern_addr_y;
    logic [15:0]    res_x, res_y;

    always_comb begin
        if (visible
            && vcount >= y_pos_16 && vcount < y_pos_16 + pattern_act_v 
            && hcount >= x_pos_16 && hcount < x_pos_16 + pattern_act_h
            ) begin
            res_x = (hcount - x_pos_16) % pattern_res_h;
            res_y = (vcount - y_pos_16) % pattern_res_v;
            //  non filped
            if (flip == 1'b0)begin
                pattern_addr_y = res_y * pattern_res_h;
                pattern_addr_x = res_x;
            end
            // filped
            else begin
                pattern_addr_y = res_y * pattern_res_h;
                pattern_addr_x = pattern_res_h - 1'b_1 - res_x;
				// pattern_addr_x = pattern_res_h - res_x;
            end
            addr_output = pattern_append + pattern_addr_y + pattern_addr_x;
            valid = 1'b1;
        end

	else if (visible
            && vcount >= y_pos_16 && vcount < y_pos_16 + pattern_act_v
            ) begin
            res_x = 16'd0;
            res_y = (vcount - y_pos_16) % pattern_res_v;
            pattern_addr_y = res_y * pattern_res_h;
            pattern_addr_x = res_x;
            addr_output = pattern_append + pattern_addr_y + pattern_addr_x;
            valid = 1'b0;
        end
        
        else begin
            addr_output = 16'd0;
            valid = 1'b0;
			pattern_addr_x = 16'd0;
			pattern_addr_y = 16'd0;
   			res_x = 16'd0;
			res_y = 16'd0;
        end
    end

   
endmodule
