module bidir_duplex_fifo_shiftregister_2048(
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
// wire [31:0] error_count;
wire [31:0] fifo_datain;
wire [31:0] fifo_dataout;
wire [31:0] pattern_to_generate;
wire [31:0] pipe_in_data;
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
assign led[6] = 1'b1;
assign led[7] = ~1'b1;

assign fifo_write_enable = pipe_in_write;
assign fifo_datain = pipe_in_data;

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

FIFO_32bit fifoForWriteTest (
	.clk(okClk),
	.rst(reset),
	.din(fifo_datain),
	.wr_en(fifo_write_enable),
	.rd_en(fifo_read_enable),
	.dout(fifo_dataout)
	// .wr_ack(wr_ack),
	// .valid(valid)
);

// checkData checkDataFromPipeIn (
// 	.data_to_check(fifo_dataout),
// 	.pattern(pattern_to_generate),
// 	.clk(okClk),
// 	.reset_err_counter(reset),
// 	.reset_pattern(reset_pattern),
// 	.check_for_errors(valid),
// 	.enable_pattern(fifo_read_enable),
// 	// .data_to_check_out(generated_data),
// 	.error_count(error_count)
// );

parameter N_WIRE_OR = 5;
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

okWireIn     ep00 (.okHE(okHE),                             .ep_addr(8'h00), .ep_dataout(pattern_to_generate));
okTriggerIn  ep40 (.okHE(okHE),                             .ep_addr(8'h40), .ep_clk(okClk), .ep_trigger(trigger));
okWireOut    ep20 (.okHE(okHE), .okEH(okEHx[ 0*65 +: 65 ]), .ep_addr(8'h20), .ep_datain(clk_counts[31:0]));
okWireOut    ep21 (.okHE(okHE), .okEH(okEHx[ 1*65 +: 65 ]), .ep_addr(8'h21), .ep_datain(clk_counts[63:32]));
okWireOut    ep22 (.okHE(okHE), .okEH(okEHx[ 2*65 +: 65 ]), .ep_addr(8'h22), .ep_datain(error_count));
okPipeIn     ep80 (.okHE(okHE), .okEH(okEHx[ 3*65 +: 65 ]), .ep_addr(8'h80), .ep_write(pipe_in_write), .ep_dataout(pipe_in_data));
okPipeOut    epa0 (.okHE(okHE), .okEH(okEHx[ 4*65 +: 65 ]), .ep_addr(8'ha0), .ep_read(pipe_out_read), .ep_datain(pipe_out_data));

endmodule