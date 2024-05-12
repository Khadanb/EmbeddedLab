module NoteGenerator (
    input wire clk,
    input wire reset,
    input wire note_enable,
    input integer half_period,
    output reg note_out
);
    integer counter = 0;

    always @(posedge clk or posedge reset) begin
        if (reset) begin
            counter <= 0;
            note_out <= 0;
        end else if (note_enable) begin
            if (counter >= half_period) begin
                note_out <= ~note_out; // Toggling output to create the square wave
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

module I2S_Controller(
    input wire clk,
    input wire reset,
    input wire [15:0] audio_data,
    output reg bclk,
    output reg ws,
    output reg sd,
    output reg ready
);
    localparam DIVIDER = 16;
    reg [7:0] bit_count = 0;
    reg [15:0] shift_reg = 0;
    integer clk_count = 0;

    always @(posedge clk or posedge reset) begin
        if (reset) begin
            clk_count <= 0;
            bclk <= 0;
            ws <= 0;
            bit_count <= 0;
            sd <= 0;
            ready <= 1;
        end else begin
            if (clk_count >= DIVIDER/2 - 1) begin
                clk_count <= 0;
                bclk <= ~bclk;
                if (bclk) begin
                    if (bit_count < 16) begin
                        sd <= shift_reg[15];
                        shift_reg <= shift_reg << 1;
                        bit_count <= bit_count + 1;
                    end else begin
                        bit_count <= 0;
                        ws <= ~ws;
                        ready <= 1;
                    end
                end
            end else begin
                clk_count <= clk_count + 1;
            end
        end
    end

    always @(posedge ws) begin
        shift_reg <= audio_data;
    end
endmodule

module AudioGenerator (
    input wire clk,
    input wire reset,
    output wire bclk,
    output wire ws,
    output wire sd
);
    localparam integer CLK_FREQ = 25000000; // 25 MHz clock
    localparam integer HP_C = CLK_FREQ / (2 * 262);
    localparam integer HP_D = CLK_FREQ / (2 * 294);
    localparam integer HP_E = CLK_FREQ / (2 * 330);
    localparam integer HP_F = CLK_FREQ / (2 * 349);
    localparam integer HP_G = CLK_FREQ / (2 * 392);
    localparam integer HP_A = CLK_FREQ / (2 * 440);
    localparam integer HP_B = CLK_FREQ / (2 * 494);
    localparam integer NOTE_DURATION = CLK_FREQ / 4; // Shorten note duration for DEMO

    localparam TOTAL_NOTES = 16;
    integer note_sequence[TOTAL_NOTES] = {E, E, E, C, E, G, G, C, G, E, A, B, B, A, G, E};
    integer durations[TOTAL_NOTES] = {NOTE_DURATION, NOTE_DURATION, NOTE_DURATION, NOTE_DURATION, NOTE_DURATION, NOTE_DURATION, NOTE_DURATION, NOTE_DURATION, NOTE_DURATION, NOTE_DURATION, NOTE_DURATION, NOTE_DURATION, NOTE_DURATION, NOTE_DURATION, NOTE_DURATION, NOTE_DURATION};

    reg [5:0] note_en = 0;
    wire [5:0] note_out;
    reg [31:0] note_timer = 0;
    integer current_note = 0;

    // Instantiate note generators
    NoteGenerator gen_c(.clk(clk), .reset(reset), .note_enable(note_en[C]), .half_period(HP_C), .note_out(note_out[C]));
    NoteGenerator gen_d(.clk(clk), .reset(reset), .note_enable(note_en[D]), .half_period(HP_D), .note_out(note_out[D]));
    NoteGenerator gen_e(.clk(clk), .reset(reset), .note_enable(note_en[E]), .half_period(HP_E), .note_out(note_out[E]));
    NoteGenerator gen_f(.clk(clk), .reset(reset), .note_enable(note_en[F]), .half_period(HP_F), .note_out(note_out[F]));
    NoteGenerator gen_g(.clk(clk), .reset(reset), .note_enable(note_en[G]), .half_period(HP_G), .note_out(note_out[G]));
    NoteGenerator gen_a(.clk(clk), .reset(reset), .note_enable(note_en[A]), .half_period(HP_A), .note_out(note_out[A]));
    NoteGenerator gen_b(.clk(clk), .reset(reset), .note_enable(note_en[B]), .half_period(HP_B), .note_out(note_out[B]));

    // Control logic for playing the song
    always @(posedge clk or posedge reset) begin
        if (reset) begin
            current_note <= 0;
            note_timer <= 0;
            note_en <= 0;
        end else begin
            if (note_timer < durations[current_note]) begin
                note_timer <= note_timer + 1;
                note_en <= 1 << note_sequence[current_note];
            end else begin
                note_timer <= 0;
                current_note <= (current_note + 1) % TOTAL_NOTES;
            end
        end
    end

    // Combine outputs to a single 16-bit sound output ==> stereo duplication
    wire [15:0] sound_data = {note_out, note_out};

    // Instantiate I2S Controller
    I2S_Controller i2s(
        .clk(clk),
        .reset(reset),
        .audio_data(sound_data),
        .bclk(bclk),
        .ws(ws),
        .sd(sd),
        .ready()
    );
endmodule

// RECALL:
// Gotta assign the bclk, ws, and sd signals to the FPGA pins that are connected to the WM8731 codec on FPGA
// TO--DO: modifying the Quartus project's pin assignments file (.qsf)