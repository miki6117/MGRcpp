module read_nonsym_fifo_blockram_1024(
	input  wire [4:0]   okUH,
	output wire [2:0]   okHU,
	inout  wire [31:0]  okUHU,
	inout  wire         okAA,

	output wire [7:0]   led
);

wire         okClk;
wire [112:0] okHE;
wire [64:0]  okEH;

reg  [63:0] clk_counts;
reg         timer_on;
wire [63:0] fifo_datain;
// wire [31:0] fifo_datain;
wire [63:0] generated_data;
// wire [31:0] generated_data;
wire [31:0] fifo_dataout;
wire [31:0] pattern_to_generate;
wire [31:0] pipe_out_data;
wire [31:0] trigger;

assign reset       = trigger[0];
assign start_timer = trigger[1];
assign stop_timer  = trigger[2];
assign reset_pattern = trigger[3];

assign led[0] = ~reset;
assign led[1] = ~start_timer;
assign led[2] = ~timer_on;
assign led[3] = ~stop_timer;
assign led[4] = ~fifo_write_enable;
assign led[5] = ~fifo_read_enable;
assign led[6] = ~fifo_empty;
assign led[7] = ~1'b1;

assign enable_gener = timer_on & ~fifo_almost_full; 
// assign fifo_datain = generated_data;
assign fifo_datain = {generated_data[31:0], generated_data[63:32]};

assign fifo_read_enable = pipe_out_read;
assign pipe_out_data = fifo_dataout;

always @(posedge okClk) begin
	if (reset) begin
		clk_counts <= 64'd0;
		timer_on   <= 0;
	end 

	if (start_timer) begin
		timer_on   <= 1;
		clk_counts <= clk_counts + 1;
	end

	if (timer_on) begin
		clk_counts <= clk_counts + 1;
	end

	if (stop_timer) begin
		timer_on   <= 0;
	end
end

FIFO_64bit fifoForReadTest (
	.wr_clk(okClk),
	.rd_clk(okClk),
	.rst(reset_pattern),
	.din(fifo_datain),
	.wr_en(fifo_write_enable),
	.rd_en(fifo_read_enable),
	.dout(fifo_dataout),
	.empty(fifo_empty),
	.almost_full(fifo_almost_full)
);

// FIFO_32bit fifoForReadTest (
//   .clk(okClk), // input clk
//   .rst(reset_pattern), // input rst
//   .din(fifo_datain), // input [31 : 0] din
//   .wr_en(fifo_write_enable), // input wr_en
//   .rd_en(fifo_read_enable), // input rd_en
//   .dout(fifo_dataout), // output [31 : 0] dout
//   .almost_full(fifo_almost_full), // output almost_full
//   .empty(fifo_empty) // output empty
// );

dataGenerator generateData (
	.pattern(pattern_to_generate),
	.clk(okClk),
	.enable_gener(enable_gener),
	.reset(reset_pattern),
	.dataout(generated_data),
	.dataout_available(fifo_write_enable)
);

// swiper generateData (
// 	.pattern(pattern_to_generate),
// 	.clk(okClk),
// 	.enable_gener(enable_gener),
// 	.reset(reset_pattern),
// 	.swiped(generated_data),
// 	.swiped_available(fifo_write_enable)
// );

// splitter generateData (
// 	.pattern(pattern_to_generate),
// 	.clk(okClk),
// 	.enable_gener(enable_gener),
// 	.reset(reset_pattern),
// 	.splitted(generated_data),
// 	.splitted_available(fifo_write_enable)
// );

parameter N_WIRE_OR = 3;
wire [65*N_WIRE_OR-1:0]  okEHx;
okHost okHI(
	.okUH(okUH),
	.okHU(okHU),
	.okUHU(okUHU),
	.okAA(okAA),
	.okClk(okClk),
	.okHE(okHE), 
	.okEH(okEH)
);

okWireOR # (.N(N_WIRE_OR)) wireOR (okEH, okEHx);

okWireIn     ep01 (.okHE(okHE),                             .ep_addr(8'h01), .ep_dataout(pattern_to_generate));
okTriggerIn  ep40 (.okHE(okHE),                             .ep_addr(8'h40), .ep_clk(okClk), .ep_trigger(trigger));
okWireOut    ep20 (.okHE(okHE), .okEH(okEHx[ 0*65 +: 65 ]), .ep_addr(8'h20), .ep_datain(clk_counts[31:0]));
okWireOut    ep21 (.okHE(okHE), .okEH(okEHx[ 1*65 +: 65 ]), .ep_addr(8'h21), .ep_datain(clk_counts[63:32]));
okPipeOut    epa0 (.okHE(okHE), .okEH(okEHx[ 2*65 +: 65 ]), .ep_addr(8'ha0), .ep_read(pipe_out_read), .ep_datain(pipe_out_data));
// okWireIn     ep00 (.okHE(okHE),                             .ep_addr(8'h00), .ep_dataout(iterations));
// okBTPipeOut  epa0 (.okHE(okHE), .okEH(okEHx[ 2*65 +: 65 ]), .ep_addr(8'ha0), .ep_datain(pipe_out_data), .ep_read(pipe_out_read), .ep_blockstrobe(pipe_out_blockstrobe), .ep_ready(pipe_out_ready));
// okWireOut    ep22 (.okHE(okHE), .okEH(okEHx[ 2*65 +: 65 ]), .ep_addr(8'h22), .ep_datain(error_count));
// okPipeIn     ep80 (.okHE(okHE), .okEH(okEHx[ 3*65 +: 65 ]), .ep_addr(8'h80), .ep_write(pipe_in_write), .ep_dataout(pipe_in_data));

endmodule
