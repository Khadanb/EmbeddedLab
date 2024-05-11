module NoteGenerator (
    input wire clk,
    input wire reset,
    input wire note_enable,
    input integer half_period,
    output reg note_out
);
    integer counter = 0;

    //square wave for a note based on the half_period
    always @(posedge clk or posedge reset) begin
        if (reset) begin
            counter <= 0;
            note_out <= 0;
        end else if (note_enable) begin
            if (counter >= half_period) begin
                note_out <= ~note_out; //toggle the output => to create the square wave
                counter <= 0;
            end else begin
                counter <= counter + 1;
            end
        end else begin
            note_out <= 0;
            counter <= 0;
        end
    end
endmodule

module AudioGenerator (
    input wire clk,           //clock at 25 MHz
    input wire reset,         //asynchronous reset
    input wire [5:0] note_en, //signals for each note (C, D, E, F, G, A)
    output wire sound_out     //combined sound output
);
    // Clock frequency
    localparam integer CLK_FREQ = 25000000; //25 MHz clock

    // Calculate half periods for each note based on their frequencies
    localparam integer HP_C = CLK_FREQ / (2 * 262); //closest integer to 261.63 Hz
    localparam integer HP_D = CLK_FREQ / (2 * 294); //" 293.66 Hz
    localparam integer HP_E = CLK_FREQ / (2 * 330); //" 329.63 Hz
    localparam integer HP_F = CLK_FREQ / (2 * 349); //" 349.23 Hz
    localparam integer HP_G = CLK_FREQ / (2 * 392); //" 392.00 Hz
    localparam integer HP_A = CLK_FREQ / (2 * 440); //" 440.00 Hz

    //instantiating note generators
    wire c_out, d_out, e_out, f_out, g_out, a_out;

    NoteGenerator gen_c (.clk(clk), .reset(reset), .note_enable(note_en[0]), .half_period(HP_C), .note_out(c_out));
    NoteGenerator gen_d (.clk(clk), .reset(reset), .note_enable(note_en[1]), .half_period(HP_D), .note_out(d_out));
    NoteGenerator gen_e (.clk(clk), .reset(reset), .note_enable(note_en[2]), .half_period(HP_E), .note_out(e_out));
    NoteGenerator gen_f (.clk(clk), .reset(reset), .note_enable(note_en[3]), .half_period(HP_F), .note_out(f_out));
    NoteGenerator gen_g (.clk(clk), .reset(reset), .note_enable(note_en[4]), .half_period(HP_G), .note_out(g_out));
    NoteGenerator gen_a (.clk(clk), .reset(reset), .note_enable(note_en[5]), .half_period(HP_A), .note_out(a_out));

    //COMBINE outputs to a single sound output
    assign sound_out = c_out | d_out | e_out | f_out | g_out | a_out;

endmodule