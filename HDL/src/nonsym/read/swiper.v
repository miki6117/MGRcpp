module swiper(
    input wire [31:0] pattern,
    input wire        clk,
    input wire        enable_gener,
    input wire        reset,

    output reg [63:0] swiped,
    output reg        swiped_available
    
);

wire [63:0] generated_data;

// always @(posedge clk) begin
//     swiped = generated_data;
//     swiped_available = generated_data_available;
// end
always @(posedge clk) begin
    if (generated_data_available) begin
        swiped_available = 1;
        // swiped = {generated_data[31:0], }
        swiped[31:0]  = generated_data[63:32];
        swiped[63:32] = generated_data[31:0];
    end else begin
        swiped_available = 0;
    end
end

dataGenerator generateData (
	.pattern(pattern),
	.clk(clk),
	.enable_gener(enable_gener),
	.reset(reset),
	.dataout(generated_data),
	.dataout_available(generated_data_available)
);

endmodule

// SPLITTER
// module splitter(
//     input wire [31:0] pattern,
//     input wire        clk,
//     input wire        enable_gener,
//     input wire        reset,

//     output reg [31:0] splitted,
//     output reg        splitted_available
    
// );

// wire [63:0] generated_data;
// reg         first_part_splitted;
// reg         second_part_splitted;

// always @(posedge clk) begin
//     if (generated_data_available & !second_part_splitted) begin
//         splitted_available = 1;
//         splitted = generated_data[31:0];
//         first_part_splitted = 1;
//     end else if (first_part_splitted) begin
//         splitted_available = 1;
//         splitted = generated_data[63:32];
//         second_part_splitted = 1;
//     // end else if (generated_data_available & second_part_splitted) begin

//     end else begin
//         splitted_available = 0;
//         first_part_splitted = 0;
//     end
//     if (reset) begin
//         first_part_splitted = 0;
//     end
// end

// dataGenerator generateData (
// 	.pattern(pattern),
// 	.clk(clk),
// 	.enable_gener(enable_gener),
// 	.reset(reset),
// 	.dataout(generated_data),
// 	.dataout_available(generated_data_available)
// );

// endmodule