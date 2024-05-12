/*
 * Avalon memory-mapped peripheral that generates VGA
 *
 * Stephen A. Edwards
 * Columbia University
 */

module ppu (
    input logic        clk,
    input logic        reset,
    input logic [31:0]  writedata,
    input logic [9:0]   hcount,
    input logic [9:0]   vcount,
    output logic [23:0] RGB_output
);

    localparam [23:0] BACKGROUND_COLOR = 24'h202020;
    logic [23:0] RGB_list[0:7];

    Mario_display Mario_0(
        .clk(clk), .reset(reset), .writedata(writedata),
        .hcount(hcount), .vcount(vcount), .RGB_output(RGB_list[0])
    );
    Goomba_display Goomba_0(
        .clk(clk), .reset(reset), .writedata(writedata),
        .hcount(hcount), .vcount(vcount), .RGB_output(RGB_list[1])
    );
    Block_display Block_0(
        .clk(clk), .reset(reset), .writedata(writedata),
        .hcount(hcount), .vcount(vcount), .RGB_output(RGB_list[2])
    );
    Fireball_display Fireball_0(
        .clk(clk), .reset(reset), .writedata(writedata),
        .hcount(hcount), .vcount(vcount), .RGB_output(RGB_list[3])
    );
    Bowser_display Bowser_0(
        .clk(clk), .reset(reset), .writedata(writedata),
        .hcount(hcount), .vcount(vcount), .RGB_output(RGB_list[4])
    );
    Peach_display Peach_0(
        .clk(clk), .reset(reset), .writedata(writedata),
        .hcount(hcount), .vcount(vcount), .RGB_output(RGB_list[5])
    );
    Ground_display Ground_0(
        .clk(clk), .reset(reset), .writedata(writedata),
        .hcount(hcount), .vcount(vcount), .RGB_output(RGB_list[6])
    );
    Tube_display Tube_0(
        .clk(clk), .reset(reset), .writedata(writedata),
        .hcount(hcount), .vcount(vcount), .RGB_output(RGB_list[7])
    );

    always_comb begin
        RGB_output = BACKGROUND_COLOR;
        for (int i = 0; i < 8; i = i + 1) begin
            if (RGB_list[i] != BACKGROUND_COLOR) begin
                RGB_output = RGB_list[i];
                break;
            end
        end
    end
endmodule

