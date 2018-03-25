module checkData(
    input wire [63:0] data_to_check,
    input wire [31:0] pattern,
    input wire        clk,
    input wire        reset_err_counter,
    input wire        reset_pattern,
    input wire        check_for_errors,
    input wire        enable_pattern,

    output wire [63:0] data_to_check_out,
    output reg [31:0] error_count
);

wire [63:0] correct_data;
// reg [63:0] reversed_data_to_check;
// wire [63:0] reversed_data_to_check;
// wire [63:0] generated_data;

assign data_to_check_out = correct_data;
// assign reversed_data_to_check = {data_to_check[31:0], data_to_check[63:32]};

always @(posedge clk) begin
    if (reset_err_counter) begin
        error_count <= 32'b0;
    end
    // if (enable_pattern) begin
    //     correct_data = generated_data;
    // end

    if (check_for_errors) begin
        // reversed_data_to_check = {data_to_check[31:0], data_to_check[63:32]};
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
    .dataout_available(dataout_available),
    .dataout(correct_data)
);

endmodule
