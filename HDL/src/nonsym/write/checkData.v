module checkData(
    input wire [63:0] data_to_check,
    input wire [31:0] pattern,
    input wire        clk,
    input wire        reset_err_counter,
    input wire        reset_pattern,
    input wire        check_for_errors,
    input wire        enable_pattern,

    output reg [31:0] error_count
);

wire [63:0] correct_data;

always @(posedge clk) begin
    if (reset_err_counter) begin
        error_count <= 32'b0;
    end

    if (check_for_errors) begin
        if (pattern != 3'b011) begin
            if(data_to_check != correct_data) begin
                error_count <= error_count + 1;
            end
        end else begin
            if(data_to_check[27:0] != correct_data[27:0]) begin
                error_count <= error_count + 1;
            end
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
