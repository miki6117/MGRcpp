module checkData(
    input wire [31:0] data_to_check,
    input wire [31:0] pattern,
    input wire        clk,
    input wire        reset_err_counter,
    input wire        reset_pattern,
    input wire        check_for_errors,
    input wire        enable_pattern,

    // output wire [31:0] data_to_check_out,
    output reg [31:0] error_count
);

wire [31:0] correct_data;
wire [31:0] generated_data;

// assign data_to_check_out = correct_data;

always @(posedge clk) begin
    if (reset_err_counter) begin
        error_count <= 32'b0;
    end
    if (check_for_errors) begin
        if(data_to_check != correct_data) begin
            error_count <= error_count + 1;
        end
    end
end

dataGenerator dataGenCheck(
    .clk(clk),
    .enable_gener(enable_pattern),
    .pattern(pattern),
    .reset(reset_pattern),
    .dataout(correct_data)
);

endmodule
